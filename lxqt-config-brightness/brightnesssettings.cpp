/*
    Copyright (C) 2016  P.L. Lucas <selairi@gmail.com>
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
    
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "brightnesssettings.h"
#include "outputwidget.h"
#include <QDebug>

BrightnessSettings::BrightnessSettings(QWidget *parent):QDialog(parent)
{
    ui = new Ui::BrightnessSettings();
    ui->setupUi(this);
    
    mBrightness = new XRandrBrightness();
    QList<MonitorInfo> monitors = mBrightness->getMonitorsInfo();

    for(MonitorInfo monitor: monitors)
    {
        OutputWidget *output = new OutputWidget(monitor, this);
        ui->layout->addWidget(output);
        output->show();
        connect(output, SIGNAL(changed(MonitorInfo)), this, SLOT(monitorSettingsChanged(MonitorInfo)));
    }
    
}

void BrightnessSettings::monitorSettingsChanged(MonitorInfo monitor)
{
    QList<MonitorInfo> monitors;
    monitors.append(monitor);
    mBrightness->setMonitorsSettings(monitors);
}

