#include "kscreenutils.h"
#include "timeoutdialog.h"
#include "xcbbackend.h"
#include "wlrbackend.h"

#include <KScreen/Output>
#include <KScreen/Mode>
#include <KScreen/Screen>
#include <KScreen/Config>
#include <KScreen/GetConfigOperation>
#include <KScreen/SetConfigOperation>
#include <KScreen/EDID>

#include <QApplication>
#include <QCryptographicHash>
#include <QMetaEnum>
#include <QTimer>
#include <QtMath>

#include <algorithm>

static bool isX11()
{
    return QGuiApplication::platformName() == QLatin1String("xcb");
}


void KScreenUtils::getConfig(std::function<void(KScreen::ConfigPtr)> callback)
{
    if (isX11()) {
        // Queued, as callers expect the result to arrive after they return.
        QTimer::singleShot(0, qApp, [callback] { callback(XcbBackend::readConfig()); });
        return;
    }
    if (WlrBackend::isAvailable()) {
        WlrBackend::readConfig(callback);
        return;
    }

    // Not leaked: libkscreen deletes the operation itself after emitting finished().
    auto *operation = new KScreen::GetConfigOperation();
    QObject::connect(operation, &KScreen::GetConfigOperation::finished, [operation, callback] {
        callback(operation->config());
    });
}


bool KScreenUtils::setConfig(const KScreen::ConfigPtr &config)
{
    if (isX11())
        return XcbBackend::applyConfig(config);
    if (WlrBackend::isAvailable())
        return WlrBackend::applyConfig(config);
    return KScreen::SetConfigOperation(config).exec();
}


bool KScreenUtils::canApply(const KScreen::ConfigPtr &config)
{
    if (isX11())
        return XcbBackend::canApply(config);
    if (WlrBackend::isAvailable())
        return WlrBackend::canApply(config);
    return KScreen::Config::canBeApplied(config);
}


void KScreenUtils::updateScreenSize(KScreen::ConfigPtr &config)
{
    const KScreen::OutputList outputs = config->outputs();
    int width, height;
    width = height = 0;
    for (const KScreen::OutputPtr &output : outputs) {
        if( !output->isConnected() )
            continue;
        QPoint pos = output->pos();
        if (output->currentMode()) {
            KScreen::ModePtr mode(output->currentMode());
            width = std::max(pos.x() + mode->size().width(), width);
            height = std::max(pos.y() + mode->size().height(), height);
        }
    }
    if (width != 0 && height != 0)
        config->screen()->setCurrentSize(QSize(width, height));
}


bool KScreenUtils::applyConfig(KScreen::ConfigPtr &config, KScreen::ConfigPtr &oldConfig)
{
    bool ok = false;
    KScreenUtils::updateScreenSize(config);
    if (config && canApply(config)) {
        const bool applied = setConfig(config);

        TimeoutDialog mTimeoutDialog;
        if (!applied || mTimeoutDialog.exec() == QDialog::Rejected) {
            KScreenUtils::updateScreenSize(oldConfig);
            setConfig(oldConfig);
        }
        else
            ok = true;
    }

    return ok;
}


void KScreenUtils::extended(KScreen::ConfigPtr &config)
{
    int width = 0;
    const KScreen::OutputList outputs = config->outputs();
    for (const KScreen::OutputPtr &output : outputs) {
        if( !output->isConnected() )
            continue;
        qDebug() << "Output: " << output->name();
        QPoint pos = output->pos();
        pos.setX(width);
        pos.setY(0);
        output->setPos(pos);
        output->setEnabled(true);
        //first left one as primary
        output->setPriority(width == 0 ? 1 : 0);
        KScreen::ModePtr mode(output->currentMode());
        //if (!mode)
        {
            // Set the biggest mode between preferred modes and the first mode.
            mode = output->modes().first();
            int modeArea = mode->size().width() * mode->size().height();
            const auto modes = output->modes();
            for(KScreen::ModePtr modeAux : modes) {
                if(output->preferredModes().contains(modeAux->id())) {
                    int modeAuxArea = modeAux->size().width() * modeAux->size().height();
                    if(modeArea < modeAuxArea) {
                        mode = modeAux;
                        modeArea = modeAuxArea;
                    }
                }
            }
            if (mode)
                output->setCurrentModeId(mode->id());
        }
        width += logicalWidth(output);
    }
}


int KScreenUtils::logicalWidth(const KScreen::OutputPtr &output)
{
    const KScreen::ModePtr mode = output->currentMode();
    if (!mode)
        return 0;
    // Positions are in logical pixels, so rotation and scale count.
    const QSize size = mode->size();
    return qCeil((output->isHorizontal() ? size.width() : size.height()) / output->scale());
}


bool KScreenUtils::supportsPrimary()
{
    return !WlrBackend::isAvailable();
}


bool KScreenUtils::supportsUnified()
{
    return !WlrBackend::isAvailable();
}


bool KScreenUtils::supportsScale()
{
    return qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
}


void KScreenUtils::allowDisabling(const KScreen::OutputPtr &output)
{
    // libkscreen 6.8 ignores setEnabled(false) without the Disable capability, which 6.7 doesn't have.
    // Looked up at run time, as libkscreen can be upgraded without rebuilding this.
    bool hasDisable = false;
    const int disable = QMetaEnum::fromType<KScreen::Output::Capability>().keyToValue("Disable", &hasDisable);
    if (hasDisable)
        output->setCapabilities(output->capabilities() | KScreen::Output::Capability(disable));
}


KScreenUtils::Identity KScreenUtils::identity(const KScreen::OutputPtr &output)
{
    const KScreen::Edid *edid = output->edid();
    if (edid && edid->isValid())
        return {edid->vendor(), edid->name(), edid->serial()};
    return WlrBackend::identity(output->name());
}


QString KScreenUtils::monitorHash(const KScreen::OutputPtr &output)
{
    const KScreen::Edid *edid = output->edid();
    if (edid && edid->isValid())
        return edid->hash();
    const Identity id = identity(output);
    if (id.vendor.isEmpty() && id.name.isEmpty() && id.serial.isEmpty())
        return QString();
    const QByteArray data = (id.vendor + u'\n' + id.name + u'\n' + id.serial).toUtf8();
    return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex());
}
