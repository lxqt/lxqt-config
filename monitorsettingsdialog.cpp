/*
    <one line to give the program's name and a brief idea of what it does.>
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
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QProcess>

#include <QGroupBox>
#include <QMessageBox>
#include "ui_monitor.h"

#include <QDebug>

#include "monitor.h"
#include "xrandr.h"


MonitorSettingsDialog::MonitorSettingsDialog():
  QDialog(NULL, 0),
  LVDS(NULL) {
  setupUi();
}


MonitorSettingsDialog::~MonitorSettingsDialog() {
}

QString MonitorSettingsDialog::humanReadableName(Monitor* monitor) {
    if(monitor == LVDS)
    return tr("Laptop LCD Monitor");
  else if( monitor->name.startsWith("VGA") || monitor->name.startsWith("Analog") )
    return LVDS ? tr("External VGA Monitor") : tr("VGA Monitor");
  else if(monitor->name.startsWith("DVI") || monitor->name.startsWith("TMDS") || monitor->name.startsWith("Digital") || monitor->name.startsWith("LVDS"))
    return LVDS ? tr("External DVI Monitor") : tr("DVI Monitor");
  else if(monitor->name.startsWith("TV") || monitor->name.startsWith("S-Video"))
    return tr("TV");
  else if(monitor->name == "default")
    return tr("Default Monitor");
  return monitor->name;
}

bool MonitorSettingsDialog::getXRandRInfo() {
  monitors = readXRandRInfo();
  foreach(Monitor *monitor, monitors) {
    if(! LVDS && ( monitor->name.startsWith("LVDS") || monitor->name.startsWith("PANEL") )  ) {
        LVDS = monitor;
        break;
    }
  }
  return true;  
}

void MonitorSettingsDialog::onResolutionChanged(int index) {
  QComboBox* combo = static_cast<QComboBox*>(sender());
  Monitor* monitor = qvariant_cast<Monitor*>(combo->property("monitor"));
  QString mode = combo->currentText();
  monitor->rateCombo->clear();
  monitor->rateCombo->addItem(tr("Auto"));
  if(monitor->modeLines.contains(mode)) {
	  QStringList mode_lines = monitor->modeLines[mode];	  
	  foreach(QString rate, mode_lines) {
	      monitor->rateCombo->addItem(rate);
	  }
	  monitor->rateCombo->setCurrentIndex(0);
  }
}

void MonitorSettingsDialog::setXRandRInfo() {
  
  QByteArray cmd = "xrandr";

  foreach(Monitor *monitor, monitors) {
    cmd += " --output ";
    cmd.append(monitor->name);
    cmd.append(' ');

    // if the monitor is turned on
    if(monitor->enable->isChecked()) {
      QString sel_res = monitor->resolutionCombo->currentText();
      QString sel_rate = monitor->rateCombo->currentText();

      if(sel_res == tr("Auto"))   // auto resolution
        cmd.append("--auto");
      else {
        cmd.append("--mode ");
        cmd.append(sel_res);

        if(sel_rate != tr("Auto") ) { // not auto refresh rate
          cmd.append(" --rate ");
          cmd.append(sel_rate);
        }
      }
    }
    else    // turn off
      cmd.append("--off");
  }
  
  
  qDebug() << "cmd:" << cmd;
  ;
  QProcess process;
  process.start(cmd);
  process.waitForFinished();
}

void MonitorSettingsDialog::chooseMaxResolution(Monitor* monitor) {
  if(monitor->resolutionCombo->count() > 1)
    monitor->resolutionCombo->setCurrentIndex(1);
}

// turn on both laptop LCD and the external monitor
void MonitorSettingsDialog::onUseBoth() {
  if(monitors.length() ==0)
    return;
  Monitor *monitor = monitors[0];
  bool ok;
  QString mode;
  for(int i=0; i<monitor->modes.length(); i++) {
    mode = monitor->modes[i];
    ok = true;
    foreach(Monitor *monitor2, monitors) {
     ok &= monitor2->modes.contains(mode);
    }
    if(ok)
      break;
  }
  qDebug() << "Mode selected" << mode << ok;
  foreach(Monitor *monitor2, monitors) {
    int index = monitor2->modes.indexOf(mode)+1;
    if(monitor2->resolutionCombo->count() > index)
      monitor2->resolutionCombo->setCurrentIndex(index);
    else
      chooseMaxResolution(monitor2);
    monitor2->enable->setChecked(true);
    qDebug() << "Mode selected index" << index << "Mode" << monitor->resolutionCombo->currentText();
  }
  accept();
}

// external monitor only
void MonitorSettingsDialog::onExternalOnly() {
  foreach(Monitor *monitor, monitors) {
    chooseMaxResolution(monitor);
    monitor->enable->setChecked(monitor != LVDS);
  }
  accept();
}

// laptop panel - LVDS only
void MonitorSettingsDialog::onLaptopOnly() {
  foreach(Monitor *monitor, monitors) {
    chooseMaxResolution(monitor);
    monitor->enable->setChecked(monitor == LVDS);
  }
  accept();
}

void MonitorSettingsDialog::setupUi() {
  ui.setupUi(this);
  connect(ui.useBoth, SIGNAL(clicked(bool)), SLOT(onUseBoth()));
  connect(ui.externalOnly, SIGNAL(clicked(bool)), SLOT(onExternalOnly()));
  connect(ui.laptopOnly, SIGNAL(clicked(bool)), SLOT(onLaptopOnly()));

  connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onDialogButtonClicked(QAbstractButton*)));
  aboutButton = new QPushButton(ui.buttonBox);
  aboutButton->setText(tr("About"));
  ui.buttonBox->addButton(aboutButton, QDialogButtonBox::HelpRole);

  getXRandRInfo();

  // If this is a laptop and there is an external monitor, offer quick options
  if(LVDS && monitors.length() == 2)
    ui.tabWidget->setCurrentIndex(0);
  else {
    ui.tabWidget->removeTab(0);
  }

  int i = 0;
  foreach(Monitor *monitor, monitors) {
    QGroupBox* box = new QGroupBox(this);
    QString title = QString("Monitor %1: %2 (%3)")
        .arg(i + 1)
        .arg(monitor->name)
        .arg(humanReadableName(monitor));
    qDebug() << "Monitor" << title;
    box->setTitle(title);
    Ui::MonitorWidget mui = Ui::MonitorWidget();
    mui.setupUi(box);
    ui.monitorLayout->insertWidget(ui.monitorLayout->count() - 1, box);
    ui.monitorLayout->setStretchFactor(box, 0);

    monitor->enable = mui.enabled;
    monitor->resolutionCombo = mui.resolution;
    monitor->resolutionCombo->setProperty("monitor", qVariantFromValue<Monitor*>(monitor));
    monitor->rateCombo = mui.rate;

    // turn off screen is not allowed since there should be at least one monitor available.
    if(monitors.length() == 1)
      monitor->enable->setEnabled(false);

    if(monitor->currentMode >= 0)
      monitor->enable->setChecked(true);

    connect(monitor->resolutionCombo, SIGNAL(currentIndexChanged(int)), SLOT(onResolutionChanged(int)));
    monitor->resolutionCombo->addItem(tr("Auto"));

    foreach(QString _mode_line, monitor->modes) {
      monitor->resolutionCombo->addItem(_mode_line);
    }
    monitor->resolutionCombo->setCurrentIndex(monitor->currentMode + 1);
    monitor->rateCombo->setCurrentIndex(monitor->currentRate + 1);
    ++i;
  }
}

void MonitorSettingsDialog::accept() {
  setXRandRInfo();
  QDialog::accept();
}

void MonitorSettingsDialog::onDialogButtonClicked(QAbstractButton* button) {
  if(ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
    setXRandRInfo();
  else if(button == aboutButton) {
    // about dialog
    QMessageBox::about(this, tr("About"), tr("LXQt-config-monitor\n\nMonitor configuration tool for LXQt."));
  }
}

