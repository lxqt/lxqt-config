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
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QJsonDocument>
#include <KScreen/EDID>




LoadSettings::LoadSettings(QObject *parent):QObject(parent)
{
    KScreen::GetConfigOperation *operation = new KScreen::GetConfigOperation();
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
    QSettings settings("LXQt", "lxqt-config-monitor");
    QJsonDocument document = QJsonDocument::fromJson( settings.value("currentConfig").toByteArray() );
    QJsonObject json = document.object();
    QJsonArray array = json["outputs"].toArray();

    applyJsonSettings(config, array);

    exit(0);
}


void applyJsonSettings(KScreen::ConfigPtr config, QJsonArray array)
{
    KScreen::OutputList outputs = config->outputs();
    for (const KScreen::OutputPtr &output : outputs)
    {
        qDebug() << "Output: " << output->name();
        for(int i=0;i<array.size();i++)
        {
            QJsonObject monitorSettings = array[i].toObject();
            if( monitorSettings["name"] == output->name() )
            {
                KScreen::Edid* edid = output->edid();
                if (edid && edid->isValid())
                    if( monitorSettings["hash"].toString() != edid->hash() )
                    {
                        qDebug() << "Hash: " << monitorSettings["hash"].toString() << "==" << edid->hash();
                        return exit(1); // Saved settings are from other monitor
                    }
                if( monitorSettings["connected"].toBool() != output->isConnected() )
                    return exit(2); // Saved settings are from other monitor
                if( !output->isConnected() )
                    continue;
                output->setEnabled( monitorSettings["enabled"].toBool() );
                output->setPrimary( monitorSettings["primary"].toBool() );
                output->setPos( QPoint(monitorSettings["xPos"].toInt(),monitorSettings["yPos"].toInt()) );
                output->setRotation( (KScreen::Output::Rotation)(monitorSettings["rotation"].toInt()) );
                // output->setCurrentModeId sometimes fails. KScreen sometimes changes mode Id.
                KScreen::ModeList modeList = output->modes();
                foreach(const KScreen::ModePtr &mode, modeList)
                {
                    QString modeSize = QString("%1x%2").arg(mode->size().width()).arg(mode->size().height());
                    if( mode->id() == monitorSettings["currentMode"].toString() 
                            ||
                            (
                                modeSize == monitorSettings["currentModeSize"].toString() 
                                    && 
                                mode->refreshRate() == monitorSettings["currentModeRate"].toDouble() 
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
}

