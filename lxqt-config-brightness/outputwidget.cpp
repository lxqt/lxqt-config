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

#include "outputwidget.h"

OutputWidget::OutputWidget(MonitorInfo monitor, QWidget *parent):QWidget(parent), mMonitor(monitor)
{
    ui = new Ui::OutputWidget();
    ui->setupUi(this);

    ui->label->setText("<b>"+monitor.name()+":</b>");
    if ( monitor.isBacklightSupported() )
    {
        ui->backlightSlider->setMinimum(0);
        ui->backlightSlider->setMaximum(monitor.backlightMax());
        ui->backlightSlider->setValue(monitor.backlight());
        ui->backlightSlider->setFocus(Qt::OtherFocusReason);
    }
    else
        ui->backlightSlider->hide();
    ui->brightnessSlider->setMinimum(0);
    ui->brightnessSlider->setMaximum(200);
    ui->brightnessSlider->setValue(monitor.brightness()*100);

    connect(ui->backlightSlider, SIGNAL(valueChanged(int)), this, SLOT(backlightChanged(int)));
    connect(ui->brightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessChanged(int)));
}

void OutputWidget::backlightChanged(int value)
{
    mMonitor.setBacklight(value);
    emit changed(mMonitor);
}


void OutputWidget::brightnessChanged(int value)
{
    mMonitor.setBrightness((float)value/100.0);
    emit changed(mMonitor);
}

void OutputWidget::setRevertedValues(const MonitorInfo & monitor)
{
    if (mMonitor.id() == monitor.id() && mMonitor.name() == monitor.name())
    {
        ui->backlightSlider->blockSignals(true);
        ui->backlightSlider->setValue(monitor.backlight());
        ui->backlightSlider->blockSignals(false);
        ui->brightnessSlider->blockSignals(true);
        ui->brightnessSlider->setValue(monitor.brightness()*100);
        ui->brightnessSlider->blockSignals(false);
    }
}
