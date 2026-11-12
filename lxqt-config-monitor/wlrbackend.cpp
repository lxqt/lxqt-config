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

#include "wlrbackend.h"

#include "qwayland-wlr-output-management-unstable-v1.h"

#include <KScreen/Mode>
#include <KScreen/Output>
#include <KScreen/Screen>

#include <QCollator>
#include <QDebug>
#include <QEventLoop>
#include <QGuiApplication>
#include <QPoint>
#include <QPointer>
#include <QSize>
#include <QTimer>
#include <QWaylandClientExtensionTemplate>

#include <wayland-client-protocol.h>

#include <algorithm>
#include <bit>
#include <memory>
#include <utility>
#include <vector>

namespace {

// make, model and serial_number need version 2, the release requests version 3.
constexpr int ManagerVersion = 4;
// How long to wait for the compositor before giving up.
constexpr int TimeoutMs = 3000;

class Mode : public QtWayland::zwlr_output_mode_v1 {
public:
    explicit Mode(struct ::zwlr_output_mode_v1 *object)
        : QtWayland::zwlr_output_mode_v1(object), id(QString::number(nextId++)) {}
    ~Mode() override
    {
        if (zwlr_output_mode_v1_get_version(object()) >= 3)
            release();
        else
            zwlr_output_mode_v1_destroy(object());
    }

    // Unique, so that a mode id in an older config can't point to another mode.
    const QString id;
    QSize size;
    int refresh = 0; // mHz
    bool preferred = false;
    bool finished = false;

protected:
    void zwlr_output_mode_v1_size(int32_t width, int32_t height) override { size = QSize(width, height); }
    void zwlr_output_mode_v1_refresh(int32_t mHz) override { refresh = mHz; }
    void zwlr_output_mode_v1_preferred() override { preferred = true; }
    void zwlr_output_mode_v1_finished() override { finished = true; }

private:
    static inline int nextId = 0;
};

class Head : public QtWayland::zwlr_output_head_v1 {
public:
    explicit Head(struct ::zwlr_output_head_v1 *object)
        : QtWayland::zwlr_output_head_v1(object) {}
    ~Head() override
    {
        if (zwlr_output_head_v1_get_version(object()) >= 3)
            release();
        else
            zwlr_output_head_v1_destroy(object());
    }

    Mode *mode(const QString &modeId) const
    {
        const auto it = std::ranges::find(modes, modeId, [](const auto &m) { return m->id; });
        return it != modes.end() ? it->get() : nullptr;
    }

    void removeFinishedModes()
    {
        if (currentMode && currentMode->finished)
            currentMode = nullptr;
        std::erase_if(modes, [](const auto &m) { return m->finished; });
    }

    QString name;
    QString make;
    QString model;
    QString serial;
    QSize physicalSize; // mm
    std::vector<std::unique_ptr<Mode>> modes;
    Mode *currentMode = nullptr;
    bool enabled = false;
    QPoint pos;
    int transform = WL_OUTPUT_TRANSFORM_NORMAL;
    qreal scale = 1;
    bool finished = false;

protected:
    void zwlr_output_head_v1_name(const QString &value) override { name = value; }
    void zwlr_output_head_v1_make(const QString &value) override { make = value; }
    void zwlr_output_head_v1_model(const QString &value) override { model = value; }
    void zwlr_output_head_v1_serial_number(const QString &value) override { serial = value; }
    void zwlr_output_head_v1_physical_size(int32_t width, int32_t height) override { physicalSize = QSize(width, height); }
    void zwlr_output_head_v1_mode(struct ::zwlr_output_mode_v1 *object) override { modes.push_back(std::make_unique<Mode>(object)); }
    void zwlr_output_head_v1_enabled(int32_t value) override
    {
        enabled = value;
        if (!enabled)
            currentMode = nullptr;
    }
    void zwlr_output_head_v1_current_mode(struct ::zwlr_output_mode_v1 *object) override
    {
        const auto it = std::ranges::find(modes, object, [](const auto &m) { return m->object(); });
        currentMode = it != modes.end() ? it->get() : nullptr;
    }
    void zwlr_output_head_v1_position(int32_t x, int32_t y) override { pos = QPoint(x, y); }
    void zwlr_output_head_v1_transform(int32_t value) override { transform = value; }
    void zwlr_output_head_v1_scale(wl_fixed_t value) override { scale = wl_fixed_to_double(value); }
    void zwlr_output_head_v1_finished() override { finished = true; }
};

class Manager : public QWaylandClientExtensionTemplate<Manager>, public QtWayland::zwlr_output_manager_v1 {
public:
    Manager() : QWaylandClientExtensionTemplate(ManagerVersion)
    {
        // Binds now if the compositor has it, instead of on the next event loop iteration.
        initialize();
    }

    Head *head(const QString &name) const
    {
        const auto it = std::ranges::find(heads, name, [](const auto &h) { return h->name; });
        return it != heads.end() ? it->get() : nullptr;
    }

    std::vector<std::unique_ptr<Head>> heads;
    uint32_t serial = 0;
    // Whether all heads have been received.
    bool ready = false;
    // Each one is called once, on the next done event.
    std::vector<std::function<void()>> doneListeners;

protected:
    void zwlr_output_manager_v1_head(struct ::zwlr_output_head_v1 *object) override { heads.push_back(std::make_unique<Head>(object)); }
    void zwlr_output_manager_v1_done(uint32_t value) override
    {
        std::erase_if(heads, [](const auto &h) { return h->finished; });
        for (const auto &h : heads)
            h->removeFinishedModes();
        serial = value;
        ready = true;
        const auto listeners = std::exchange(doneListeners, {});
        for (const auto &listener : listeners)
            listener();
    }
    void zwlr_output_manager_v1_finished() override
    {
        // The compositor destroyed the manager, so nothing can be read or applied any more.
        ready = false;
        heads.clear();
        zwlr_output_manager_v1_destroy(object());
    }
};

struct DestroyConfigHead {
    void operator()(struct ::zwlr_output_configuration_head_v1 *head) const { zwlr_output_configuration_head_v1_destroy(head); }
};
using ConfigHead = std::unique_ptr<struct ::zwlr_output_configuration_head_v1, DestroyConfigHead>;

class Configuration : public QtWayland::zwlr_output_configuration_v1 {
public:
    enum class Result { Pending, Succeeded, Failed, Cancelled };

    explicit Configuration(struct ::zwlr_output_configuration_v1 *object)
        : QtWayland::zwlr_output_configuration_v1(object) {}
    // Also destroys the configured heads on the compositor side.
    ~Configuration() override { destroy(); }

    /** Sends apply or test, and waits for the answer of the compositor.
     */
    Result send(bool testOnly)
    {
        testOnly ? test() : apply();
        QEventLoop loop;
        mLoop = &loop;
        QTimer::singleShot(TimeoutMs, &loop, &QEventLoop::quit);
        loop.exec();
        mLoop = nullptr;
        return mResult;
    }

    std::vector<ConfigHead> heads;

protected:
    void zwlr_output_configuration_v1_succeeded() override { finish(Result::Succeeded); }
    void zwlr_output_configuration_v1_failed() override { finish(Result::Failed); }
    void zwlr_output_configuration_v1_cancelled() override { finish(Result::Cancelled); }

private:
    void finish(Result result)
    {
        mResult = result;
        if (mLoop)
            mLoop->quit();
    }

    QEventLoop *mLoop = nullptr;
    Result mResult = Result::Pending;
};

static Manager *manager()
{
    // Never deleted, as it lives as long as the Wayland connection.
    // Only created on Wayland, as QWaylandClientExtension expects the Wayland platform plugin.
    static Manager *m = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>() ? new Manager : nullptr;
    return m && m->isActive() ? m : nullptr;
}

/** Runs the event loop until the next done event, for TimeoutMs at most. */
static void waitForDone(Manager *m)
{
    QEventLoop loop;
    m->doneListeners.push_back([l = QPointer<QEventLoop>(&loop)] {
        if (l)
            l->quit();
    });
    QTimer::singleShot(TimeoutMs, &loop, &QEventLoop::quit);
    loop.exec();
}

// KScreen rotations are 1 << wl_output transform.
static KScreen::Output::Rotation rotationOf(int transform)
{
    return KScreen::Output::Rotation(1 << transform);
}

static int transformOf(KScreen::Output::Rotation rotation)
{
    const auto value = static_cast<unsigned>(rotation);
    return std::has_single_bit(value) && value <= KScreen::Output::Flipped270 ? std::countr_zero(value) : -1;
}

static KScreen::OutputPtr outputNamed(const KScreen::ConfigPtr &config, const QString &name)
{
    const KScreen::OutputList outputs = config->outputs();
    const auto it = std::ranges::find(outputs, name, [](const KScreen::OutputPtr &o) { return o->name(); });
    return it != outputs.end() ? *it : nullptr;
}

static bool isBuiltIn(const QString &name)
{
    return name.startsWith(u"eDP") || name.startsWith(u"LVDS") || name.startsWith(u"DSI");
}

static KScreen::ConfigPtr configFrom(const Manager *m)
{
    // The dialog lists outputs by id, and compositors announce heads in no fixed order.
    // Built-in panels come first, then the others by name, like on X11.
    std::vector<const Head *> heads;
    for (const auto &head : m->heads)
        heads.push_back(head.get());
    QCollator collator;
    collator.setNumericMode(true);
    std::ranges::sort(heads, [&collator](const Head *a, const Head *b) {
        if (isBuiltIn(a->name) != isBuiltIn(b->name))
            return isBuiltIn(a->name);
        return collator.compare(a->name, b->name) < 0;
    });

    KScreen::OutputList outputs;
    int id = 0;
    for (const Head *head : heads) {
        KScreen::OutputPtr output(new KScreen::Output);
        output->setId(id++);
        output->setName(head->name);
        // Heads only exist while their monitor is connected.
        output->setConnected(true);
        KScreenUtils::allowDisabling(output);
        output->setSizeMm(head->physicalSize);

        KScreen::ModeList modes;
        QStringList preferred;
        for (const auto &m : head->modes) {
            KScreen::ModePtr mode(new KScreen::Mode);
            mode->setId(m->id);
            mode->setName(QStringLiteral("%1x%2").arg(m->size.width()).arg(m->size.height()));
            mode->setSize(m->size);
            mode->setRefreshRate(m->refresh / 1000.0f);
            modes.insert(mode->id(), mode);
            if (m->preferred)
                preferred.append(mode->id());
        }
        output->setModes(modes);
        output->setPreferredModes(preferred);

        output->setEnabled(head->enabled);
        if (head->enabled) {
            // Not set for custom modes, which aren't in the list.
            if (head->currentMode)
                output->setCurrentModeId(head->currentMode->id);
            output->setPos(head->pos);
            output->setRotation(rotationOf(head->transform));
            output->setScale(head->scale);
        }
        outputs.insert(output->id(), output);
    }

    KScreen::ScreenPtr screen(new KScreen::Screen);
    screen->setMaxActiveOutputsCount(int(m->heads.size()));

    KScreen::ConfigPtr config(new KScreen::Config);
    config->setScreen(screen);
    config->setOutputs(outputs);
    return config;
}

static Configuration::Result send(Manager *m, const KScreen::ConfigPtr &config, bool testOnly)
{
    Configuration configuration(m->create_configuration(m->serial));
    for (const auto &head : m->heads) {
        // Every head has to be configured, so the ones that config doesn't have keep their state.
        const KScreen::OutputPtr output = outputNamed(config, head->name);
        if (output ? !output->isEnabled() : !head->enabled) {
            configuration.disable_head(head->object());
            continue;
        }

        Mode *mode = head->currentMode;
        int transform = head->transform;
        QPoint pos = head->pos;
        qreal scale = head->scale;
        if (output) {
            // An empty mode id is a custom mode, which is kept.
            if (!output->currentModeId().isEmpty())
                mode = head->mode(output->currentModeId());
            transform = transformOf(output->rotation());
            // Reflections can't be chosen in the UI, so a current one is kept.
            if (transform >= 0 && transform < WL_OUTPUT_TRANSFORM_FLIPPED && head->transform >= WL_OUTPUT_TRANSFORM_FLIPPED)
                transform += WL_OUTPUT_TRANSFORM_FLIPPED;
            pos = output->pos();
            scale = output->scale();
        }
        if ((output && !output->currentModeId().isEmpty() && !mode) || transform < 0 || scale <= 0) {
            qDebug() << "lxqt-config-monitor: Invalid mode, rotation or scale for" << head->name;
            return Configuration::Result::Failed;
        }

        ConfigHead configHead(configuration.enable_head(head->object()));
        if (mode)
            zwlr_output_configuration_head_v1_set_mode(configHead.get(), mode->object());
        zwlr_output_configuration_head_v1_set_position(configHead.get(), pos.x(), pos.y());
        zwlr_output_configuration_head_v1_set_transform(configHead.get(), transform);
        zwlr_output_configuration_head_v1_set_scale(configHead.get(), wl_fixed_from_double(scale));
        configuration.heads.push_back(std::move(configHead));
    }
    return configuration.send(testOnly);
}

static bool sendConfig(const KScreen::ConfigPtr &config, bool testOnly)
{
    Manager *m = manager();
    if (!m || !m->ready || !config)
        return false;

    const uint32_t serial = m->serial;
    Configuration::Result result = send(m, config, testOnly);
    if (result == Configuration::Result::Cancelled && m->ready) {
        // The monitors changed, or the result of an earlier apply wasn't received yet. Retried with the new state.
        if (m->serial == serial)
            waitForDone(m);
        result = send(m, config, testOnly);
    }
    if (result != Configuration::Result::Succeeded)
        qDebug() << "lxqt-config-monitor: The compositor refused the settings, result" << int(result);
    return result == Configuration::Result::Succeeded;
}

} // namespace

bool WlrBackend::isAvailable()
{
    return manager();
}

void WlrBackend::readConfig(std::function<void(KScreen::ConfigPtr)> callback)
{
    // Queued, as callers expect the result to arrive after they return.
    QTimer::singleShot(0, qApp, [callback] {
        Manager *m = manager();
        // Just bound: the compositor sends the heads, then done.
        if (m && !m->ready)
            waitForDone(m);
        callback(m && m->ready ? configFrom(m) : nullptr);
    });
}

bool WlrBackend::canApply(const KScreen::ConfigPtr &config)
{
    return sendConfig(config, true);
}

bool WlrBackend::applyConfig(const KScreen::ConfigPtr &config)
{
    return sendConfig(config, false);
}

KScreenUtils::Identity WlrBackend::identity(const QString &outputName)
{
    const Manager *m = manager();
    const Head *head = m ? m->head(outputName) : nullptr;
    return head ? KScreenUtils::Identity{head->make, head->model, head->serial} : KScreenUtils::Identity{};
}
