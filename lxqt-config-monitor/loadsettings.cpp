/*
    Copyright (C) 2015  P.L. Lucas <selairi@gmail.com>

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




#include "loadsettings.h"
#include "kscreenutils.h"
#include <KScreen/Output>
#include <KScreen/Config>
#include <KScreen/Mode>
#include <LXQt/Settings>
#include <LXQt/Notification>
#include <QCoreApplication>


LoadSettings::LoadSettings(QObject *parent):QObject(parent)
{
    mNotification = new LXQt::Notification(QLatin1String(""), this);
}

QList<MonitorSettings> LoadSettings::loadCurrentConfiguration()
{
    LXQt::Settings settings(QStringLiteral("lxqt-config-monitor"));
    QList<MonitorSettings> monitors;
    settings.beginGroup(QStringLiteral("currentConfig"));
    loadMonitorSettings(settings, monitors);
    settings.endGroup();

    return monitors;
}

QList<MonitorSettings> LoadSettings::loadConfiguration(QString scope)
{
    LXQt::Settings settings(QStringLiteral("lxqt-config-monitor"));
    QList<MonitorSettings> monitors;
    settings.beginGroup(scope);
    loadMonitorSettings(settings, monitors);
    settings.endGroup();

    return monitors;
}

void LoadSettings::applyBestSettings()
{
    KScreenUtils::getConfig([this] (KScreen::ConfigPtr config) {
        bool ok = false;
        if (config != nullptr) {
            qDebug() << "lxqt-config-monitor: Applying current settings...";
            QList<MonitorSettings> monitors = loadConfiguration(QStringLiteral("currentConfig"));
            ok = applySettings(config, monitors);
            if(!ok) {
                qDebug() << "lxqt-config-monitor: Current settings can not be applied.";
                qDebug() << "lxqt-config-monitor: Searching in saved settings...";
                // Load configs
                LXQt::Settings settings(QStringLiteral("lxqt-config-monitor"));
                QList<MonitorSavedSettings> monitorsSavedSettings;
                settings.beginGroup(QStringLiteral("SavedConfigs"));
                loadMonitorSettings(settings, monitorsSavedSettings);
                settings.endGroup();
                for(const MonitorSavedSettings& o : std::as_const(monitorsSavedSettings)) {
                    ok = applySettings(config, o.monitors);
                    if(ok) break;
                }
                if(!ok) {
                    // Saved configs can not be applied
                    // Extended mode will be applied
                    qDebug() << "lxqt-config-monitor: No saved settings has been found";
                    KScreenUtils::extended(config);
                    KScreenUtils::updateScreenSize(config);
                    ok = KScreenUtils::canApply(config) && KScreenUtils::setConfig(config);
                    if (ok) {
                        qDebug() << "lxqt-config-monitor: Extended mode has been applied";

                        mNotification->setSummary(tr("Default monitor settings applied."));
                        mNotification->setBody(tr("Adjust your screens with LXQt Monitor Settings."));
                        mNotification->setIcon(QStringLiteral("preferences-desktop-display"));
                        mNotification->update();
                        mNotification->setTimeout(1000);
                        mNotification->setUrgencyHint(LXQt::Notification::UrgencyLow);
                    }
                }
            }
        }
        if(ok)
            qDebug() << tr("lxqt-config-monitor: Settings applied.");
        else
            qDebug().noquote() << tr("lxqt-config-monitor: Settings not applied.\nIf this is a Wayland compositor, you could try kanshi to configure your monitor(s).");
        QCoreApplication::instance()->exit(0);
    });
}


bool applySettings(KScreen::ConfigPtr config, QList<MonitorSettings> monitors)
{
    const KScreen::OutputList outputs = config->outputs();
    if(outputs.size() != monitors.size())
        return false;
    for (const KScreen::OutputPtr &output : outputs) {
        qDebug() << "Output: " << output->name();
        bool outputFound = false;
        for(int i=0; i<monitors.size() && !outputFound; i++) {
            MonitorSettings monitor = monitors[i];
            if( monitor.name == output->name() ) {
                outputFound = true;
                if (!KScreenUtils::canBeMonitor(monitor.hash, output)) {
                    qDebug() << "Hash: " << monitor.hash << "!=" << KScreenUtils::monitorHash(output);
                    return false; // Saved settings are from other monitor
                }
                if( monitor.connected != output->isConnected() )
                    return false; // Saved settings are from other monitor
                if( !output->isConnected() )
                    continue;
                output->setEnabled( monitor.enabled );
                output->setPriority( monitor.primary ? 1 : 0 );
                output->setPos( QPoint(monitor.xPos, monitor.yPos) );
                output->setRotation( (KScreen::Output::Rotation)(monitor.rotation) );
                // Settings saved before scales were saved keep the current scale.
                if (monitor.scale > 0)
                    output->setScale(monitor.scale);
                // output->setCurrentModeId could fail. KScreen sometimes changes mode Id.
                KScreen::ModeList modeList = output->modes();
                for(const KScreen::ModePtr &mode : std::as_const(modeList)) {
                    if( mode->id() == QString(monitor.currentMode)
                            ||
                            (
                                mode->size().width() == monitor.currentModeWidth
                                &&
                                mode->size().height() == monitor.currentModeHeight
                                &&
                                mode->refreshRate() == monitor.currentModeRate
                            )
                      ) {
                        output->setCurrentModeId( mode->id() );
                        break;
                    }
                }

            }
        }
        if(!outputFound)
            return false;
    }

    KScreenUtils::updateScreenSize(config);
    return KScreenUtils::canApply(config) && KScreenUtils::setConfig(config);
}

