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

#include "screen_listener.h"

#include <KScreen/GetConfigOperation>
#include <KScreen/SetConfigOperation>
#include <KScreen/ConfigMonitor>
#include <QProcess>
#include <QDebug>

ScreenListener::ScreenListener(QObject *parent) : QObject(parent)
{
    configMonitor = NULL;
    timer = new QTimer(this);
    timer->setInterval(30000);
    connect(timer, SIGNAL(timeout()), this, SLOT(testOutputChanged()));
    init();

    // KScreen::GetConfigOperation *operation = new KScreen::GetConfigOperation();
    // connect(operation, &KScreen::GetConfigOperation::finished, [this, operation] (KScreen::ConfigOperation *op) {
    //     KScreen::GetConfigOperation *configOp = qobject_cast<KScreen::GetConfigOperation *>(op);
    //     if (configOp)
    //     {
    //         config = configOp->config();
    //         init();
    //         operation->deleteLater();
    //     }
    // });
}

void ScreenListener::init()
{
    qDebug() << "Conecting signals";
    KScreen::ConfigMonitor *configMonitor = KScreen::ConfigMonitor::instance();
    connect(configMonitor, SIGNAL(configurationChanged()), this, SLOT(configurationChanged()));
    timer->start();
}


void ScreenListener::configurationChanged()
{
    qDebug() << "Configuration Changed";
    QProcess::execute("lxqt-config-monitor");
}

void ScreenListener::testOutputChanged()
{
    // Write xrandr config for debug.
    qDebug() << "testOutputChanged";
    QProcess::execute("xrandr");
}

