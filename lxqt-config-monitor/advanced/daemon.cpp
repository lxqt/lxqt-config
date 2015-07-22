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

#include "daemon.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>

DaemonSettings::DaemonSettings(LxQt::Settings *settings, QWidget *parent): QWidget(parent)
{
    this->settings = settings;

    ui.setupUi(this);

    settings->beginGroup("daemon");
    ui.autostartCheckBox->setChecked( settings->value("autostart", QVariant(false) ).toBool() );
    settings->endGroup();

    connect(ui.autostartCheckBox, SIGNAL(toggled(bool)), this, SLOT(setAutostart(bool)));
    connect(ui.startButton, SIGNAL(clicked()), this, SLOT(startDaemon()));
    connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(stopDaemon()));
}


void DaemonSettings::stopDaemon()
{
    QProcess::execute("killall lxqt-config-monitor-daemon");
}

void DaemonSettings::startDaemon()
{
    if( ! QProcess::startDetached("lxqt-config-monitor-daemon") )
        QMessageBox::critical(this, tr("Error"), tr("Daemon can not be started"));
}

void DaemonSettings::setAutostart(bool autostart)
{
    settings->beginGroup("daemon");
    settings->setValue("autostart", autostart);
    settings->endGroup();

    if(autostart)
    {
        QString desktop = QString("[Desktop Entry]\n"
                                  "Type=Application\n"
                                  "Name=LXQt-config-monitor autostart\n"
                                  "Comment=Autostart monitor settings for LXQt-config-monitor\n"
                                  "Exec=%1\n"
                                  "OnlyShowIn=LXQt\n").arg("lxqt-config-monitor-daemon");
        // Check if ~/.config/autostart/ exists
        bool ok = true;
        QFileInfo fileInfo(QDir::homePath() + "/.config/autostart/");
        if( ! fileInfo.exists() )
            ok = QDir::root().mkpath(QDir::homePath() + "/.config/autostart/");
        QFile file(QDir::homePath() + "/.config/autostart/lxqt-config-monitor-daemon-autostart.desktop");
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
    else
    {
        QFile file(QDir::homePath() + "/.config/autostart/lxqt-config-monitor-daemon-autostart.desktop");
        
        if(file.exists())
            file.remove();
    }
}
