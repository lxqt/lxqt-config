/*
    This file is part of the LXQt project.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "xcbbackend.h"
#include "kscreenutils.h"

#include <KScreen/Mode>
#include <KScreen/Output>
#include <KScreen/Screen>

#include <QDebug>
#include <QGuiApplication>
#include <QList>
#include <QMap>
#include <QPoint>
#include <QSet>
#include <QSize>

#include <xcb/randr.h>

#include <algorithm>
#include <bit>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace {

struct FreeReply {
    void operator()(void *reply) const { free(reply); }
};
template <typename T> using Reply = std::unique_ptr<T, FreeReply>;
using Resources = Reply<xcb_randr_get_screen_resources_current_reply_t>;
using OutputInfo = Reply<xcb_randr_get_output_info_reply_t>;
using CrtcInfo = Reply<xcb_randr_get_crtc_info_reply_t>;

struct Placement {
    xcb_randr_crtc_t crtc;
    xcb_randr_mode_t mode;
    QPoint pos;
    uint16_t rotation;
    QList<xcb_randr_output_t> outputs;
};

struct Plan {
    QMap<xcb_randr_crtc_t, Placement> placements;
    QSize screenSize;
};

static xcb_connection_t *connection()
{
    auto *x11 = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    return x11 ? x11->connection() : nullptr;
}

static xcb_screen_t *rootScreen(xcb_connection_t *c)
{
    return xcb_setup_roots_iterator(xcb_get_setup(c)).data;
}

static Resources screenResources(xcb_connection_t *c)
{
    Reply<xcb_randr_query_version_reply_t> version(xcb_randr_query_version_reply(c, xcb_randr_query_version(c, 1, 3), nullptr));
    if (!version || version->major_version < 1 || (version->major_version == 1 && version->minor_version < 3)) {
        qWarning() << "lxqt-config-monitor: RandR 1.3 or later is required";
        return nullptr;
    }
    // The cheap request below only returns cached data, and at login nobody may have filled that cache yet.
    static bool cacheFilled = false;
    if (!cacheFilled) {
        Reply<xcb_randr_get_screen_resources_reply_t> fill(xcb_randr_get_screen_resources_reply(c, xcb_randr_get_screen_resources(c, rootScreen(c)->root), nullptr));
        cacheFilled = true;
    }
    return Resources(xcb_randr_get_screen_resources_current_reply(c, xcb_randr_get_screen_resources_current(c, rootScreen(c)->root), nullptr));
}

static OutputInfo outputInfo(xcb_connection_t *c, xcb_randr_output_t output, xcb_timestamp_t timestamp)
{
    return OutputInfo(xcb_randr_get_output_info_reply(c, xcb_randr_get_output_info(c, output, timestamp), nullptr));
}

static CrtcInfo crtcInfo(xcb_connection_t *c, xcb_randr_crtc_t crtc, xcb_timestamp_t timestamp)
{
    return CrtcInfo(xcb_randr_get_crtc_info_reply(c, xcb_randr_get_crtc_info(c, crtc, timestamp), nullptr));
}

static std::span<xcb_randr_output_t> outputsOf(const Resources &resources)
{
    return {xcb_randr_get_screen_resources_current_outputs(resources.get()),
            size_t(xcb_randr_get_screen_resources_current_outputs_length(resources.get()))};
}

static std::span<xcb_randr_crtc_t> crtcsOf(const Resources &resources)
{
    return {xcb_randr_get_screen_resources_current_crtcs(resources.get()),
            size_t(xcb_randr_get_screen_resources_current_crtcs_length(resources.get()))};
}

static std::span<xcb_randr_mode_t> modesOf(const OutputInfo &info)
{
    return {xcb_randr_get_output_info_modes(info.get()), size_t(xcb_randr_get_output_info_modes_length(info.get()))};
}

static std::span<xcb_randr_output_t> clonesOf(const OutputInfo &info)
{
    return {xcb_randr_get_output_info_clones(info.get()), size_t(xcb_randr_get_output_info_clones_length(info.get()))};
}

static std::span<xcb_randr_crtc_t> crtcsOf(const OutputInfo &info)
{
    return {xcb_randr_get_output_info_crtcs(info.get()), size_t(xcb_randr_get_output_info_crtcs_length(info.get()))};
}

template <typename T>
static bool contains(std::span<T> values, T value)
{
    return std::find(values.begin(), values.end(), value) != values.end();
}

// Must match what libkscreen reports, because saved settings compare the refresh rate exactly.
static float refreshRate(const xcb_randr_mode_info_t &mode)
{
    if (mode.htotal == 0 || mode.vtotal == 0)
        return 0;
    double vTotal = mode.vtotal;
    if (mode.mode_flags & XCB_RANDR_MODE_FLAG_DOUBLE_SCAN)
        vTotal *= 2;
    if (mode.mode_flags & XCB_RANDR_MODE_FLAG_INTERLACE)
        vTotal /= 2;
    return float(mode.dot_clock) / (float(mode.htotal) * vTotal);
}

static QByteArray edidOf(xcb_connection_t *c, xcb_randr_output_t output)
{
    for (const char *name : {"EDID", "EDID_DATA", "XFree86_DDC_EDID1_RAWDATA"}) {
        Reply<xcb_intern_atom_reply_t> atom(xcb_intern_atom_reply(c, xcb_intern_atom(c, 0, strlen(name), name), nullptr));
        if (!atom)
            continue;
        // 100 is what libkscreen reads: the EDID hash in saved settings depends on it.
        Reply<xcb_randr_get_output_property_reply_t> property(xcb_randr_get_output_property_reply(c,
            xcb_randr_get_output_property(c, output, atom->atom, XCB_ATOM_ANY, 0, 100, false, false), nullptr));
        if (property && property->format == 8 && property->num_items > 0 && property->num_items % 128 == 0)
            return QByteArray(reinterpret_cast<const char *>(xcb_randr_get_output_property_data(property.get())), property->num_items);
    }
    return QByteArray();
}

static KScreen::ModeList modesFrom(const Resources &resources, const OutputInfo &info, QStringList &preferred)
{
    const auto *modeInfos = xcb_randr_get_screen_resources_current_modes(resources.get());
    const int modeInfoCount = xcb_randr_get_screen_resources_current_modes_length(resources.get());
    const auto modeIds = modesOf(info);

    KScreen::ModeList modes;
    for (size_t i = 0; i < modeIds.size(); ++i) {
        const auto *end = modeInfos + modeInfoCount;
        const auto *modeInfo = std::find_if(modeInfos, end, [&](const auto &m) { return m.id == modeIds[i]; });
        if (modeInfo == end)
            continue;
        KScreen::ModePtr mode(new KScreen::Mode);
        mode->setId(QString::number(modeInfo->id));
        mode->setName(QStringLiteral("%1x%2").arg(modeInfo->width).arg(modeInfo->height));
        mode->setSize(QSize(modeInfo->width, modeInfo->height));
        mode->setRefreshRate(refreshRate(*modeInfo));
        modes.insert(mode->id(), mode);
        if (int(i) < info->num_preferred)
            preferred.append(mode->id());
    }
    return modes;
}

static KScreen::OutputPtr outputFrom(xcb_connection_t *c, const Resources &resources, xcb_randr_output_t id, xcb_randr_output_t primary)
{
    OutputInfo info = outputInfo(c, id, resources->config_timestamp);
    if (!info)
        return nullptr;

    KScreen::OutputPtr output(new KScreen::Output);
    output->setId(id);
    KScreenUtils::allowDisabling(output);
    output->setName(QString::fromUtf8(reinterpret_cast<const char *>(xcb_randr_get_output_info_name(info.get())),
                                      xcb_randr_get_output_info_name_length(info.get())));
    // Config::primaryOutput() is the lowest priority, so everything else has to be above 1.
    output->setPriority(id == primary ? 1 : 2);
    output->setConnected(info->connection == XCB_RANDR_CONNECTION_CONNECTED);
    if (!output->isConnected())
        return output;

    QStringList preferred;
    output->setModes(modesFrom(resources, info, preferred));
    output->setPreferredModes(preferred);
    output->setSizeMm(QSize(info->mm_width, info->mm_height));
    output->setEdid(edidOf(c, id));

    CrtcInfo crtc = info->crtc != XCB_NONE ? crtcInfo(c, info->crtc, resources->config_timestamp) : nullptr;
    output->setEnabled(crtc && crtc->mode != XCB_NONE);
    if (output->isEnabled()) {
        output->setCurrentModeId(QString::number(crtc->mode));
        output->setPos(QPoint(crtc->x, crtc->y));
        output->setRotation(KScreen::Output::Rotation(crtc->rotation & 0xf));
        output->setSize(QSize(crtc->width, crtc->height));
    }
    return output;
}

/** Decides which CRTC drives which output, without changing anything. */
static std::optional<Plan> makePlan(xcb_connection_t *c, const Resources &resources, const KScreen::ConfigPtr &config)
{
    struct Enabled {
        KScreen::OutputPtr output;
        OutputInfo info;
    };
    std::vector<Enabled> enabled;
    QPoint origin(INT_MAX, INT_MAX);
    const KScreen::OutputList outputs = config->outputs();
    for (const KScreen::OutputPtr &output : outputs) {
        if (!output->isConnected() || !output->isEnabled())
            continue;
        OutputInfo info = outputInfo(c, output->id(), resources->config_timestamp);
        if (!info || !output->currentMode()) {
            qWarning() << "lxqt-config-monitor: no mode to turn on" << output->name() << "with";
            return std::nullopt;
        }
        origin = QPoint(std::min(origin.x(), output->pos().x()), std::min(origin.y(), output->pos().y()));
        enabled.push_back({output, std::move(info)});
    }
    if (enabled.empty()) {
        qWarning() << "lxqt-config-monitor: refusing to turn off every monitor";
        return std::nullopt;
    }

    // Outputs keep their CRTC when possible, so they go first to stop others from taking it.
    std::ranges::stable_sort(enabled, [](const Enabled &a, const Enabled &b) {
        return a.info->crtc != XCB_NONE && b.info->crtc == XCB_NONE;
    });

    Plan plan;
    for (const Enabled &e : enabled) {
        const xcb_randr_mode_t mode = e.output->currentModeId().toUInt();
        if (!contains(modesOf(e.info), mode)) {
            qWarning() << "lxqt-config-monitor: mode" << mode << "is not available on" << e.output->name();
            return std::nullopt;
        }

        const QPoint pos = e.output->pos() - origin;
        // KScreen has no reflection, so a reflection set outside LXQt is kept.
        CrtcInfo current = e.info->crtc != XCB_NONE ? crtcInfo(c, e.info->crtc, resources->config_timestamp) : nullptr;
        const uint16_t rotation = e.output->rotation()
            | (current ? current->rotation & (XCB_RANDR_ROTATION_REFLECT_X | XCB_RANDR_ROTATION_REFLECT_Y) : 0);
        const auto possible = crtcsOf(e.info);

        // Mirrored outputs share a CRTC only when they are clones of each other, as the X server refuses it otherwise.
        const auto clones = clonesOf(e.info);
        auto shared = std::find_if(plan.placements.begin(), plan.placements.end(), [&](const Placement &p) {
            return p.mode == mode && p.pos == pos && p.rotation == rotation && contains(possible, p.crtc)
                && std::all_of(p.outputs.cbegin(), p.outputs.cend(), [&](xcb_randr_output_t x) { return contains(clones, x); });
        });
        if (shared != plan.placements.end()) {
            shared->outputs.append(e.output->id());
            continue;
        }

        xcb_randr_crtc_t crtc = e.info->crtc;
        if (crtc == XCB_NONE || plan.placements.contains(crtc)) {
            auto freeCrtc = std::ranges::find_if(possible, [&](xcb_randr_crtc_t x) { return !plan.placements.contains(x); });
            if (freeCrtc == possible.end()) {
                qWarning() << "lxqt-config-monitor: no free CRTC for" << e.output->name();
                return std::nullopt;
            }
            crtc = *freeCrtc;
        }
        CrtcInfo info = crtcInfo(c, crtc, resources->config_timestamp);
        // Exactly one rotation, as RandR rejects anything else; 0 comes from settings saved without one.
        if (!info || std::popcount(rotation & 0xfu) != 1 || (info->rotations & rotation) != rotation) {
            qWarning() << "lxqt-config-monitor: rotation" << rotation << "is not supported on" << e.output->name();
            return std::nullopt;
        }
        plan.placements.insert(crtc, Placement{crtc, mode, pos, rotation, {xcb_randr_output_t(e.output->id())}});

        QSize size = e.output->currentMode()->size();
        if (rotation & (XCB_RANDR_ROTATION_ROTATE_90 | XCB_RANDR_ROTATION_ROTATE_270))
            size.transpose();
        plan.screenSize = plan.screenSize.expandedTo(QSize(pos.x() + size.width(), pos.y() + size.height()));
    }

    Reply<xcb_randr_get_screen_size_range_reply_t> range(xcb_randr_get_screen_size_range_reply(c,
        xcb_randr_get_screen_size_range(c, rootScreen(c)->root), nullptr));
    if (!range || plan.screenSize.width() > range->max_width || plan.screenSize.height() > range->max_height) {
        qWarning() << "lxqt-config-monitor: screen size" << plan.screenSize << "is above the maximum";
        return std::nullopt;
    }
    plan.screenSize = plan.screenSize.expandedTo(QSize(range->min_width, range->min_height));
    return plan;
}

static bool sameOutputs(const CrtcInfo &current, const Placement &wanted)
{
    const std::span<xcb_randr_output_t> outputs(xcb_randr_get_crtc_info_outputs(current.get()), xcb_randr_get_crtc_info_outputs_length(current.get()));
    return QSet<xcb_randr_output_t>(outputs.begin(), outputs.end()) == QSet<xcb_randr_output_t>(wanted.outputs.begin(), wanted.outputs.end());
}

static bool unchanged(const CrtcInfo &current, const Placement &wanted)
{
    return current->mode == wanted.mode && current->rotation == wanted.rotation
        && QPoint(current->x, current->y) == wanted.pos && sameOutputs(current, wanted);
}

static bool setCrtc(xcb_connection_t *c, const Resources &resources, const Placement &p)
{
    Reply<xcb_randr_set_crtc_config_reply_t> reply(xcb_randr_set_crtc_config_reply(c,
        xcb_randr_set_crtc_config(c, p.crtc, XCB_CURRENT_TIME, resources->config_timestamp, p.pos.x(), p.pos.y(), p.mode, p.rotation,
                                  p.outputs.size(), p.outputs.constData()), nullptr));
    return reply && reply->status == XCB_RANDR_SET_CONFIG_SUCCESS;
}

} // namespace

KScreen::ConfigPtr XcbBackend::readConfig()
{
    xcb_connection_t *c = connection();
    Resources resources = c ? screenResources(c) : nullptr;
    if (!resources)
        return nullptr;

    xcb_screen_t *root = rootScreen(c);
    Reply<xcb_randr_get_screen_size_range_reply_t> range(xcb_randr_get_screen_size_range_reply(c,
        xcb_randr_get_screen_size_range(c, root->root), nullptr));
    Reply<xcb_randr_get_output_primary_reply_t> primary(xcb_randr_get_output_primary_reply(c,
        xcb_randr_get_output_primary(c, root->root), nullptr));
    if (!range || !primary)
        return nullptr;

    KScreen::ScreenPtr screen(new KScreen::Screen);
    screen->setMinSize(QSize(range->min_width, range->min_height));
    screen->setMaxSize(QSize(range->max_width, range->max_height));
    screen->setCurrentSize(QSize(root->width_in_pixels, root->height_in_pixels));
    screen->setMaxActiveOutputsCount(crtcsOf(resources).size());

    KScreen::OutputList outputs;
    for (xcb_randr_output_t id : outputsOf(resources)) {
        if (KScreen::OutputPtr output = outputFrom(c, resources, id, primary->output))
            outputs.insert(output->id(), output);
    }

    KScreen::ConfigPtr config(new KScreen::Config);
    config->setScreen(screen);
    config->setOutputs(outputs);
    return config;
}

bool XcbBackend::canApply(const KScreen::ConfigPtr &config)
{
    xcb_connection_t *c = connection();
    Resources resources = c && config ? screenResources(c) : nullptr;
    return resources && makePlan(c, resources, config).has_value();
}

bool XcbBackend::applyConfig(const KScreen::ConfigPtr &config)
{
    xcb_connection_t *c = connection();
    Resources resources = c && config ? screenResources(c) : nullptr;
    if (!resources)
        return false;
    const std::optional<Plan> plan = makePlan(c, resources, config);
    if (!plan)
        return false;

    xcb_screen_t *root = rootScreen(c);
    bool ok = true;
    xcb_grab_server(c);

    // Off first: CRTCs that go away or change outputs, and ones outside the new size, which would make the resize fail.
    // The others are changed in place, so they don't blank.
    for (xcb_randr_crtc_t crtc : crtcsOf(resources)) {
        CrtcInfo current = crtcInfo(c, crtc, resources->config_timestamp);
        if (!current || current->mode == XCB_NONE)
            continue;
        const auto wanted = plan->placements.constFind(crtc);
        const bool fits = current->x + current->width <= plan->screenSize.width()
            && current->y + current->height <= plan->screenSize.height();
        if (wanted == plan->placements.cend() || !sameOutputs(current, *wanted) || !fits)
            if (!setCrtc(c, resources, Placement{crtc, XCB_NONE, QPoint(0, 0), XCB_RANDR_ROTATION_ROTATE_0, {}})) {
                qWarning() << "lxqt-config-monitor: could not turn off CRTC" << crtc;
                ok = false;
            }
    }

    // Resizing stops all drawing on the screen, even for the same size.
    Reply<xcb_get_geometry_reply_t> geometry(xcb_get_geometry_reply(c, xcb_get_geometry(c, root->root), nullptr));
    if (!geometry || plan->screenSize != QSize(geometry->width, geometry->height)) {
        const xcb_void_cookie_t resized = xcb_randr_set_screen_size_checked(c, root->root,
            plan->screenSize.width(), plan->screenSize.height(),
            std::max(1, plan->screenSize.width() * root->width_in_millimeters / root->width_in_pixels),
            std::max(1, plan->screenSize.height() * root->height_in_millimeters / root->height_in_pixels));
        if (Reply<xcb_generic_error_t> error{xcb_request_check(c, resized)}) {
            qWarning() << "lxqt-config-monitor: could not resize the screen to" << plan->screenSize << "error" << error->error_code;
            ok = false;
        }
    }

    for (const Placement &wanted : plan->placements) {
        CrtcInfo current = crtcInfo(c, wanted.crtc, resources->config_timestamp);
        if (!current || !unchanged(current, wanted))
            if (!setCrtc(c, resources, wanted)) {
                qWarning() << "lxqt-config-monitor: could not set CRTC" << wanted.crtc << "to mode" << wanted.mode << "at" << wanted.pos;
                ok = false;
            }
    }

    xcb_randr_output_t primary = XCB_NONE;
    const KScreen::OutputList outputs = config->outputs();
    for (const KScreen::OutputPtr &output : outputs) {
        if (output->isConnected() && output->isEnabled() && output->priority() == 1)
            primary = output->id();
    }
    xcb_randr_set_output_primary(c, root->root, primary);

    xcb_ungrab_server(c);
    xcb_flush(c);
    if (!ok)
        qWarning() << "lxqt-config-monitor: RandR rejected the configuration";
    return ok;
}
