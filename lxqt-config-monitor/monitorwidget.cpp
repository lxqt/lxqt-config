/*
    Copyright (C) 2014  P.L. Lucas <selairi@gmail.com>
    Copyright (C) 2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

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

#include "monitorwidget.h"
#include "monitor.h"
#include <QDebug>

MonitorWidget::MonitorWidget(MonitorInfo* monitor, const QList<MonitorInfo*> monitorsInfo, QWidget* parent):
  QGroupBox(parent) {
  ui.enabled = NULL;
  monitorInfo = monitor;
  monitor->setParent(this); // take the ownership

  ui.setupUi(this);

  if(monitorsInfo.length() == 1) {
    disablePositionOption(true);

    // turn off screen is not allowed since there should be at least one monitor available.
    ui.enabled->setEnabled(false);
  }
  
  ui.xPosSpinBox->setValue(monitor->xPos);
  ui.yPosSpinBox->setValue(monitor->yPos);

  if(monitor->enabledOk)
    ui.enabled->setChecked(true);

  connect(ui.resolutionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onResolutionChanged(int)));
  ui.resolutionCombo->addItem(tr("Auto"));
  Q_FOREACH(QString _mode_line, monitor->modes) {
    QVariant monitorModeInfo = QVariant::fromValue(monitor->monitorModes[_mode_line]);
    ui.resolutionCombo->addItem(_mode_line, monitorModeInfo);
  }
  
 
  if(!monitor->currentMode.isEmpty())
    ui.resolutionCombo->setCurrentIndex(ui.resolutionCombo->findText(monitor->currentMode));
  else
    ui.resolutionCombo->setCurrentIndex(0);
  if(!monitor->currentRate.isEmpty())
    ui.rateCombo->setCurrentIndex(ui.rateCombo->findText(monitor->currentRate));
  else
     ui.rateCombo->setCurrentIndex(0);
  
  int brightness;
  if( !monitorInfo->brightness.isEmpty() )
    brightness = monitorInfo->brightness.toFloat()*100;
  else
    brightness = 100;
  ui.brightnessSlider->setValue(brightness);
  
  // Set gamma values
  ui.redSpinBox->setSingleStep(0.01);
  ui.greenSpinBox->setSingleStep(0.01);
  ui.blueSpinBox->setSingleStep(0.01);
  if(!monitor->gamma.isEmpty()) {
    QStringList gammaValues = monitor->gamma.split(":");
    ui.redSpinBox->setValue(gammaValues[0].toFloat());
    ui.greenSpinBox->setValue(gammaValues[1].toFloat());
    ui.blueSpinBox->setValue(gammaValues[2].toFloat());
  }
  
  //Set backlight values
  if( !monitor->backlight.isEmpty() ) {
    ui.backlightSlider->setMinimum(monitor->backlightMin.toInt());
    ui.backlightSlider->setMaximum(monitor->backlightMax.toInt());
    ui.backlightSlider->setSingleStep(1);
    ui.backlightSlider->setValue(monitor->backlight.toInt());
  } else {
    ui.backlightSlider->setEnabled(false);
    ui.backlightLabel->setEnabled(false);
  }
}

void MonitorWidget::onResolutionChanged(int index) {
  QComboBox* combo =ui.resolutionCombo;
  QComboBox* rateCombo = ui.rateCombo;
  QString mode = combo->currentText();
  rateCombo->clear();
  rateCombo->addItem(tr("Auto"));
  if( monitorInfo->monitorModes.contains(mode)) {
    QStringList mode_lines = combo->currentData().value<MonitorMode*>()->modeLines;
    //QStringList mode_lines = monitorInfo->monitorModes[mode]->modeLines;
    Q_FOREACH(QString rate, mode_lines) {
      rateCombo->addItem(rate);
    }
    rateCombo->setCurrentIndex(0);
  }
}


void MonitorWidget::disablePositionOption(bool disable) {
  bool enable = !disable;
  ui.xPosSpinBox->setEnabled(enable);
  ui.yPosSpinBox->setEnabled(enable);
  ui.xPosLabel->setEnabled(enable);
  ui.yPosLabel->setEnabled(enable);
  ui.positionLabel->setEnabled(enable);
}

MonitorSettings* MonitorWidget::getSettings() {
  MonitorSettings* s = new MonitorSettings();
  s->name = monitorInfo->name;
  s->enabledOk = ui.enabled->isChecked();
  s->currentMode = ui.resolutionCombo->currentText();
  s->currentRate = ui.rateCombo->currentText();
  if( ! ui.xPosSpinBox->isEnabled() ) { // If no unify monitor is selected, then position is disabled.
    s->position = MonitorSettings::None;
  } else {
    s->position = MonitorSettings::Manual;
  }
  s->xPos=ui.xPosSpinBox->value();
  s->yPos=ui.yPosSpinBox->value();
  s->brightness = QString("%1").arg((float)(ui.brightnessSlider->value())/100.0);
  s->gamma = QString("%1:%2:%3").arg(ui.redSpinBox->value()).arg(ui.greenSpinBox->value()).arg(ui.blueSpinBox->value());
  if(ui.backlightSlider->isEnabled()) {
    s->backlight = QString("%1").arg(ui.backlightSlider->value());
    s->backlightMax = QString("%1").arg(ui.backlightSlider->maximum());
    s->backlightMin = QString("%1").arg(ui.backlightSlider->minimum());
  }
  return s;
}

void MonitorWidget::chooseMaxResolution() {
  if(ui.resolutionCombo->count() > 1)
    ui.resolutionCombo->setCurrentIndex(1);
}

void MonitorWidget::enableMonitor(bool enable) {
  ui.enabled->setChecked(enable);
}
