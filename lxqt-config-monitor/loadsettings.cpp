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
#include <KScreen/Output>
#include <KScreen/Config>
#include <KScreen/GetConfigOperation>
#include <KScreen/SetConfigOperation>
#include <LXQt/Settings>
#include <KScreen/EDID>
#include <QCoreApplication>


LoadSettings::LoadSettings(QObject *parent):QObject(parent)
{
    KScreen::GetConfigOperation *operation  = new KScreen::GetConfigOperation();
    connect(operation, &KScreen::GetConfigOperation::finished, [this, operation] (KScreen::ConfigOperation *op) {
        KScreen::GetConfigOperation *configOp = qobject_cast<KScreen::GetConfigOperation *>(op);
        if (configOp)
        {
            loadConfiguration(configOp->config());
            operation->deleteLater();
        }
    });
}

void LoadSettings::loadConfiguration(KScreen::ConfigPtr config)
{
    LXQt::Settings settings("lxqt-config-monitor");
    QList<MonitorSettings> monitors;
    settings.beginGroup("currentConfig");
    loadMonitorSettings(settings, monitors);
    settings.endGroup();

    applySettings(config, monitors);
}


void applySettings(KScreen::ConfigPtr config, QList<MonitorSettings> monitors)
{
    KScreen::OutputList outputs = config->outputs();
    for (const KScreen::OutputPtr &output : outputs)
    {
        qDebug() << "Output: " << output->name();
        for(int i=0;i<monitors.size();i++)
        {
            MonitorSettings monitor = monitors[i];
            if( monitor.name == output->name() )
            {
                KScreen::Edid* edid = output->edid();
                if (edid && edid->isValid())
                    if( monitor.hash != edid->hash() )
                    {
                        qDebug() << "Hash: " << monitor.hash << "==" << edid->hash();
                        return QCoreApplication::instance()->exit(1); // Saved settings are from other monitor
                    }
                if( monitor.connected != output->isConnected() )
                    return QCoreApplication::instance()->exit(2); // Saved settings are from other monitor
                if( !output->isConnected() )
                    continue;
                output->setEnabled( monitor.enabled );
                output->setPrimary( monitor.primary );
                output->setPos( QPoint(monitor.xPos, monitor.yPos) );
                output->setRotation( (KScreen::Output::Rotation)(monitor.rotation) );
                // output->setCurrentModeId could fail. KScreen sometimes changes mode Id.
                KScreen::ModeList modeList = output->modes();
                foreach(const KScreen::ModePtr &mode, modeList)
                {
                    if( mode->id() == QString(monitor.currentMode) 
                            ||
                            (
                                mode->size().width() == monitor.currentModeWidth
                                    &&
                                mode->size().height() == monitor.currentModeHeight 
                                    && 
                                mode->refreshRate() == monitor.currentModeRate 
                            )
                      )
                    {
                        output->setCurrentModeId( mode->id() );
                        break;
                    }
                }

            }
        }
    }

    if (KScreen::Config::canBeApplied(config))
        KScreen::SetConfigOperation(config).exec();

    QCoreApplication::instance()->exit(0);
}

