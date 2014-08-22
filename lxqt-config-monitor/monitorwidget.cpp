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

MonitorWidget::MonitorWidget(MonitorInfo* monitor, const QList<MonitorInfo*> monitorsInfo, QWidget* parent):
  QGroupBox(parent) {
  ui.enabled = NULL;
  monitorInfo = monitor;
  monitor->setParent(this); // take the ownership

  ui.setupUi(this);
  ui.resolutionCombo->setProperty("modeLines", qVariantFromValue<QHash<QString, QStringList> >(monitor->modeLines));
  ui.resolutionCombo->setProperty("rateCombo", qVariantFromValue<QComboBox*>(ui.rateCombo));

  ui.relativeToOutputCombo->addItem(tr("Default"));
  Q_FOREACH(MonitorInfo * other, monitorsInfo) {
    if(other != monitor) {
      ui.relativeToOutputCombo->addItem(other->name);
      if(monitor->positionRelativeToOutput == other->name)
        ui.relativeToOutputCombo->setCurrentIndex(ui.relativeToOutputCombo->count() - 1);
    }
  }
  ui.positionCombo->setCurrentIndex(monitor->position);

  if(monitorsInfo.length() == 1) {
    ui.positionCombo->setEnabled(false);
    ui.relativeToOutputCombo->setEnabled(false);
    ui.positionLabel->setEnabled(false);

    // turn off screen is not allowed since there should be at least one monitor available.
    ui.enabled->setEnabled(false);
  }

  if(monitor->enabledOk)
    ui.enabled->setChecked(true);

  // FIXME: this is dirty
  connect(ui.resolutionCombo, SIGNAL(currentIndexChanged(int)), parent, SLOT(onResolutionChanged(int)));
  ui.resolutionCombo->addItem(tr("Auto"));

  Q_FOREACH(QString _mode_line, monitor->modes) {
    ui.resolutionCombo->addItem(_mode_line);
  }
  ui.resolutionCombo->setCurrentIndex(ui.resolutionCombo->findText(monitor->currentMode));
  ui.rateCombo->setCurrentIndex(ui.rateCombo->findText(monitor->currentRate));
}

void MonitorWidget::disablePositionOption(bool disable) {
  bool enable = !disable;
  ui.positionCombo->setEnabled(enable);
  ui.relativeToOutputCombo->setEnabled(enable);
  ui.positionLabel->setEnabled(enable);
}

MonitorSettings* MonitorWidget::getSettings() {
  MonitorSettings* s = new MonitorSettings();
  s->name = monitorInfo->name;
  s->enabledOk = ui.enabled->isChecked();
  s->currentMode = ui.resolutionCombo->currentText();
  s->currentRate = ui.rateCombo->currentText();
  s->position = (MonitorSettings::Position)ui.positionCombo->currentIndex();
  s->positionRelativeToOutput = ui.relativeToOutputCombo->currentText();
  return s;
}

void MonitorWidget::chooseMaxResolution() {
  if(ui.resolutionCombo->count() > 1)
    ui.resolutionCombo->setCurrentIndex(1);
}

void MonitorWidget::enableMonitor(bool enable) {
  ui.enabled->setChecked(enable);
}
