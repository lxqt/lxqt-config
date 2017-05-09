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
#include <QMessageBox>
#include <QPushButton>

BrightnessSettings::BrightnessSettings(QWidget *parent):QDialog(parent)
{
    ui = new Ui::BrightnessSettings();
    ui->setupUi(this);
    
    mBrightness = new XRandrBrightness();
    mMonitors = mBrightness->getMonitorsInfo();

    for(const MonitorInfo &monitor: mMonitors)
    {
        OutputWidget *output = new OutputWidget(monitor, this);
        ui->layout->addWidget(output);
        output->show();
        connect(output, SIGNAL(changed(MonitorInfo)), this, SLOT(monitorSettingsChanged(MonitorInfo)));
        connect(this, &BrightnessSettings::monitorReverted, output, &OutputWidget::setRevertedValues);
    }

    mConfirmRequestTimer.setSingleShot(true);
    mConfirmRequestTimer.setInterval(1000);
    connect(&mConfirmRequestTimer, &QTimer::timeout, this, &BrightnessSettings::requestConfirmation);
    
}

void BrightnessSettings::monitorSettingsChanged(MonitorInfo monitor)
{
    mBrightness->setMonitorsSettings(QList<MonitorInfo>{}  << monitor);
    if (ui->confirmCB->isChecked())
    {
        mConfirmRequestTimer.start();
    } else
    {
        for (auto & m : mMonitors)
        {
            if (m.id() == monitor.id() && m.name() == monitor.name())
            {
                m.setBacklight(monitor.backlight());
                m.setBrightness(monitor.brightness());
            }
        }
    }
}

void BrightnessSettings::requestConfirmation()
{
    QMessageBox msg{QMessageBox::Question, tr("Brightness settings changed")
        , tr("Confirmation required. Are the settings correct?")
        , QMessageBox::Yes | QMessageBox::No};
    int timeout = 5; // seconds
    QString no_text = msg.button(QMessageBox::No)->text();
    no_text += QStringLiteral("(%1)");
    msg.setButtonText(QMessageBox::No, no_text.arg(timeout));
    msg.setDefaultButton(QMessageBox::No);

    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(false);
    timeoutTimer.setInterval(1000);
    connect(&timeoutTimer, &QTimer::timeout, [&] {
        msg.setButtonText(QMessageBox::No, no_text.arg(--timeout));
        if (timeout == 0)
        {
            timeoutTimer.stop();
            msg.reject();
        }
    });
    timeoutTimer.start();

    if (QMessageBox::Yes == msg.exec())
    {
        // re-read current values
        mMonitors = mBrightness->getMonitorsInfo();
    } else
    {
        // revert the changes
        mBrightness->setMonitorsSettings(mMonitors);
        for (const auto & monitor : mMonitors)
            emit monitorReverted(monitor);
    }
}
