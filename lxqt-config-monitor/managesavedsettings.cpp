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

#include <QJsonArray>
#include <QJsonObject>
#include "managesavedsettings.h"
#include "loadsettings.h"
#include "configure.h"
#include <QDebug>
#include <QJsonDocument>
#include <QInputDialog>
#include <QDateTime>


ManageSavedSettings::ManageSavedSettings(LXQt::Settings * applicationSettings,  KScreen::ConfigPtr config, QWidget * parent):
QDialog(parent)
{

    this->applicationSettings = applicationSettings;
    this->config = config;

    ui.setupUi(this);

    connect(ui.allConfigs, SIGNAL(itemActivated(QListWidgetItem *)), SLOT(showSelectedConfig(QListWidgetItem *)));
    connect(ui.deletePushButton, SIGNAL(clicked()), SLOT(onDeleteItem()));
    connect(ui.renamePushButton, SIGNAL(clicked()), SLOT(onRenameItem()));
    connect(ui.applyPushButton, SIGNAL(clicked()), SLOT(onApplyItem()));

    loadSettings();
}


void ManageSavedSettings::showSelectedConfig(QListWidgetItem * item)
{
    QJsonObject o = item->data(Qt::UserRole).toJsonObject();
    QJsonArray jsonArray = o["outputs"].toArray();
    QString text;
    for(int i=0; i < jsonArray.size(); i++)
    {
        QJsonObject setting = jsonArray[i].toObject();
        if(setting["connected"].toBool() == false)
            continue;
        text += QString("<b>%1:</b><br/>").arg(setting["name"].toString());
        text += QString("&nbsp;Mode: %1<br/>").arg(setting["currentModeSize"].toString());
        switch(setting["rotation"].toInt())
        {
            case KScreen::Output::Rotation::None:
                text += QString("&nbsp;Rotation: %1<br/>").arg(tr("None"));
                break;
            case KScreen::Output::Rotation::Left:
                text += QString("&nbsp;Rotation: %1<br/>").arg(tr("Left"));
                break;
            case KScreen::Output::Rotation::Inverted:
                text += QString("&nbsp;Rotation: %1<br/>").arg(tr("Inverted"));
                break;
            case KScreen::Output::Rotation::Right:
                text += QString("&nbsp;Rotation: %1<br/>").arg(tr("Right"));
                break;
        }
        text += QString("&nbsp;Position: %1x%2<br/>").arg(setting["xPos"].toInt()).arg(setting["yPos"].toInt());
        text += QString("&nbsp;Primary: %1<br/>").arg(setting["primary"].toBool()?tr("True"):tr("False"));
        text += QString("&nbsp;Enabled: %1<br/>").arg(setting["enabled"].toBool()?tr("True"):tr("False"));
    }
    text += "<br/>";
    ui.selectedSettingsTextEdit->setText(text);
}


bool ManageSavedSettings::isHardwareCompatible(QJsonObject json)
{
    KScreen::OutputList outputs = config->outputs();
    QJsonArray jsonArray = json["outputs"].toArray();
    for (const KScreen::OutputPtr &output : outputs)
    {
        bool ok = false;
        for (int i=0; i < jsonArray.size(); i++)
        {
            const QJsonValue & v = jsonArray[i];
            QJsonObject o = v.toObject();
            if(o["name"] != output->name())
                continue;
            KScreen::Edid *edid = output->edid();
            if(edid && edid->isValid())
                if(o["hash"] != output->edid()->hash())
                    return false;
            ok = true;
            break;
        }
        if(!ok)
            return false;
    }
    return true;
}

void ManageSavedSettings::onDeleteItem()
{
    if (ui.allConfigs->currentItem() == NULL)
        return;
    QJsonObject obj = ui.allConfigs->currentItem()->data(Qt::UserRole).toJsonObject();
    QSettings settings("LXQt", "lxqt-config-monitor");
    QJsonDocument document = QJsonDocument::fromJson(settings.value("SavedConfigs").toByteArray());
    QJsonObject json = document.object();
    QJsonArray jsonArray = json["configs"].toArray();
    for (int i = 0; i < jsonArray.size(); i++) {
        const QJsonValue & v = jsonArray[i];
        QJsonObject o = v.toObject();
        if (o == obj) {
            jsonArray.removeAt(i);
            break;
        }
    }
    json["configs"] = jsonArray;
    settings.setValue("SavedConfigs", QVariant(QJsonDocument(json).toJson()));

    loadSettings();
}

void ManageSavedSettings::onRenameItem()
{
    if (ui.allConfigs->currentItem() == NULL)
        return;
    QJsonObject obj = ui.allConfigs->currentItem()->data(Qt::UserRole).toJsonObject();
    bool ok;
    QString configName = QInputDialog::getText(this, tr("Name"), tr("Name:"),
                                               QLineEdit::Normal,
                                               obj["name"].toString(), &ok);
    if (!ok || configName.isEmpty())
        return;

    QSettings settings("LXQt", "lxqt-config-monitor");
    QJsonDocument document = QJsonDocument::fromJson(settings.value("SavedConfigs").toByteArray());
    QJsonObject json = document.object();
    QJsonArray jsonArray = json["configs"].toArray();
    for (int i = 0; i < jsonArray.size(); i++) {
        const QJsonValue & v = jsonArray[i];
        QJsonObject o = v.toObject();
        if (o == obj) {
            jsonArray.removeAt(i);
            obj["name"] = configName;
            jsonArray.append(obj);
            break;
        }
    }
    json["configs"] = jsonArray;
    settings.setValue("SavedConfigs", QVariant(QJsonDocument(json).toJson()));

    loadSettings();
}

void ManageSavedSettings::onApplyItem()
{
    if (ui.allConfigs->currentItem() == NULL)
        return;
    QJsonObject json = ui.allConfigs->currentItem()->data(Qt::UserRole).toJsonObject();
    applyJsonSettings(config, json["outputs"].toArray());
}



void ManageSavedSettings::loadSettings()
{
    ui.allConfigs->clear();
    //ui.hardwareCompatibleConfigs->clear();
    QSettings settings("LXQt", "lxqt-config-monitor");
    QJsonDocument document = QJsonDocument::fromJson(settings.value("SavedConfigs").toByteArray());
    QJsonArray savedConfigs = document.object()["configs"].toArray();
    foreach(const QJsonValue & v, savedConfigs) {
        QJsonObject o = v.toObject();
        QListWidgetItem *item = new QListWidgetItem(o["name"].toString()+" - "+o["date"].toString(), ui.allConfigs);
        item->setData(Qt::UserRole, QVariant(o));
        if (isHardwareCompatible(o)) {
            //QListWidgetItem *item1 = new QListWidgetItem(o["name"].toString(),
            //                                            ui.hardwareCompatibleConfigs);
            //item1->setData(Qt::UserRole, QVariant(o));
            QFont font = ui.allConfigs->font();
            font.setBold(true);
            item->setData(Qt::FontRole, font);
        }
    }
}
