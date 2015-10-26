/*
    Copyright (C) 2014  P.L. Lucas <selairi@gmail.com>
    Copyright (C) 2013  <copyright holder> <email>

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

#include "monitorsettingsdialog.h"

#include "monitorwidget.h"
#include "timeoutdialog.h"
#include "monitorpicture.h"
#include "settingsdialog.h"

#include <KScreen/Output>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QJsonDocument>
#include <KScreen/EDID>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

MonitorSettingsDialog::MonitorSettingsDialog() :
    QDialog(nullptr, 0)
{
    ui.setupUi(this);

    KScreen::GetConfigOperation *operation = new KScreen::GetConfigOperation();
    connect(operation, &KScreen::GetConfigOperation::finished, [this, operation] (KScreen::ConfigOperation *op) {
        KScreen::GetConfigOperation *configOp = qobject_cast<KScreen::GetConfigOperation *>(op);
        if (configOp)
        {
            mOldConfig = configOp->config()->clone();
            loadConfiguration(configOp->config());
            operation->deleteLater();
        }
    });

    connect(ui.buttonBox, &QDialogButtonBox::clicked, [&] (QAbstractButton *button) {
        if (ui.buttonBox->standardButton(button) == QDialogButtonBox::Apply)
            applyConfiguration(false);
        if (ui.buttonBox->standardButton(button) == QDialogButtonBox::Save)
        {
            applyConfiguration(true);
        }

    });

    ui.settingsButton->hide(); // Hide settings button until daemon works OK.
    connect(ui.settingsButton, SIGNAL(clicked()), this, SLOT(showSettingsDialog()));
}

MonitorSettingsDialog::~MonitorSettingsDialog()
{
}

void MonitorSettingsDialog::loadConfiguration(KScreen::ConfigPtr config)
{
    if (mConfig == config)
        return;

    mConfig = config;

    MonitorPictureDialog *monitorPicture = nullptr;
    KScreen::OutputList outputs = mConfig->outputs();

    int nMonitors = 0;
    for (const KScreen::OutputPtr &output : outputs)
    {
        if (output->isConnected())
            nMonitors++;

        if (nMonitors > 1)
        {
            monitorPicture = new MonitorPictureDialog(config, this);
            ui.monitorList->addItem(tr("Set position"));
            ui.stackedWidget->addWidget(monitorPicture);
            break;
        }
    }

    QList<MonitorWidget*> monitors;

    for (const KScreen::OutputPtr &output : outputs)
    {
        if (output->isConnected())
        {
            MonitorWidget *monitor = new MonitorWidget(output, mConfig, this);
            ui.monitorList->addItem(output->name());
            ui.stackedWidget->addWidget(monitor);
            monitors.append(monitor);
        }
    }

    if (monitorPicture)
        monitorPicture->setScene(monitors);

    ui.monitorList->setCurrentRow(0);
    adjustSize();
}

void MonitorSettingsDialog::applyConfiguration(bool saveConfigOk)
{
    if (mConfig && KScreen::Config::canBeApplied(mConfig))
    {
        KScreen::SetConfigOperation(mConfig).exec();

        TimeoutDialog mTimeoutDialog;
        if (mTimeoutDialog.exec() == QDialog::Rejected)
            KScreen::SetConfigOperation(mOldConfig).exec();
        else
        {
            mOldConfig = mConfig->clone();
            if (saveConfigOk)
                saveConfiguration(mConfig);
        }
    }
}

void MonitorSettingsDialog::accept()
{
    //applyConfiguration(true);
    QDialog::accept();
}

void MonitorSettingsDialog::reject()
{
    QDialog::reject();
}

void MonitorSettingsDialog::saveConfiguration(KScreen::ConfigPtr config)
{
    QJsonObject json;
    QJsonArray jsonArray;
    KScreen::OutputList outputs = config->outputs();
    for (const KScreen::OutputPtr &output : outputs)
    {
        QJsonObject monitorSettings;
        monitorSettings["name"] = output->name();
        KScreen::Edid* edid = output->edid();
        if (edid && edid->isValid())
            monitorSettings["hash"] = edid->hash();
        monitorSettings["connected"] = output->isConnected();
        if ( output->isConnected() )
        {
            monitorSettings["enabled"] = output->isEnabled();
            monitorSettings["primary"] = output->isPrimary();
            monitorSettings["xPos"] = output->pos().x();
            monitorSettings["yPos"] = output->pos().y();
            monitorSettings["currentMode"] = output->currentMode()->id();
            monitorSettings["rotation"] = output->rotation();
        }
        jsonArray.append(monitorSettings);
    }
    json["outputs"] = jsonArray;
    
    QSettings settings("LXQt", "lxqt-config-monitor");
    settings.setValue("currentConfig", QVariant(QJsonDocument(json).toJson()));

    // Check if autostart file exists. It is commented because of old configs.
    //QFileInfo desktopFileInfo(QDir::homePath() + "/.config/autostart/lxqt-config-monitor-autostart.desktop");
    //if( desktopFileInfo.exists() )
    //    return;


    QString desktop = QString("[Desktop Entry]\n"
                              "Type=Application\n"
                              "Name=LXQt-config-monitor autostart\n"
                              "Comment=Autostart monitor settings for LXQt-config-monitor\n"
                              "Exec=%1\n"
                              "OnlyShowIn=LXQt\n").arg("lxqt-config-monitor -l");
    // Check if ~/.config/autostart/ exists
    bool ok = true;
    QFileInfo fileInfo(QDir::homePath() + "/.config/autostart/");
    if( ! fileInfo.exists() )
      ok = QDir::root().mkpath(QDir::homePath() + "/.config/autostart/");
    QFile file(QDir::homePath() + "/.config/autostart/lxqt-config-monitor-autostart.desktop");
    if(ok)
            ok = file.open(QIODevice::WriteOnly | QIODevice::Text);
    if(!ok) {
      QMessageBox::critical(this, tr("Error"), tr("Config can not be saved"));
      return;
    }
    QTextStream out(&file);
    out << desktop;
    out.flush();
    file.close();

}

void MonitorSettingsDialog::showSettingsDialog()
{
    QByteArray configName = qgetenv("LXQT_SESSION_CONFIG");

    if (configName.isEmpty())
        configName = "MonitorSettings";

    LXQt::Settings settings(configName);

    SettingsDialog settingsDialog(tr("Advanced settings"), &settings);
    settingsDialog.exec();
}
