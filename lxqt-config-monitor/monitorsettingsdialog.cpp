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
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QProcess>
#include <QGroupBox>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include "ui_monitor.h"

#include <QDebug>

#include "monitor.h"
#include "xrandr.h"


MonitorSettingsDialog::MonitorSettingsDialog(Backend *backend):
  QDialog(NULL, 0),
  LVDS(NULL) {
  timeoutDialog = NULL;
  timer = NULL;
  this->backend = backend;
  backend->setParent(this);
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

void MonitorSettingsDialog::onUnifyChanged(int index) {
  foreach(Monitor *monitor, monitors) {
    monitor->positionCombo->setEnabled( ! ui.unify->isChecked());
    monitor->relativeToOutputCombo->setEnabled( ! ui.unify->isChecked());
    monitor->positionLabel->setEnabled( ! ui.unify->isChecked());
  }
  ui.primaryCombo->setEnabled( ! ui.unify->isChecked());
  ui.primaryLabel->setEnabled( ! ui.unify->isChecked());
}

void MonitorSettingsDialog::onResolutionChanged(int index) {
  QComboBox* combo = static_cast<QComboBox*>(sender());
  QHash<QString, QStringList> modeLines = qvariant_cast<QHash<QString, QStringList> >(combo->property("modeLines"));
  QComboBox *rateCombo = qvariant_cast<QComboBox*>(combo->property("rateCombo"));
  QString mode = combo->currentText();
  rateCombo->clear();
  rateCombo->addItem(tr("Auto"));
  if(modeLines.contains(mode)) {
	  QStringList mode_lines = modeLines[mode];	  
	  foreach(QString rate, mode_lines) {
	      rateCombo->addItem(rate);
	  }
	  rateCombo->setCurrentIndex(0);
  }
}


void MonitorSettingsDialog::deleteTimeoutData() {
  if(timer!=NULL) {
    timer->stop();
    delete timer;
    timer = NULL;
  }   
  if(timeoutDialog!=NULL) {
   delete timeoutDialog;
   timeoutDialog = NULL;
  }
  foreach(MonitorInfo *monitorInfo, timeoutSettings) {
    delete monitorInfo;
  }
  timeoutSettings.clear();
}

void MonitorSettingsDialog::onCancelSettings() {
  deleteTimeoutData();
}

void MonitorSettingsDialog::onTimeout() {
  int time = timeoutDialog->value()+1;
  if(time>=10) { // If time is finished, settings are restored.
    timer->stop();
    QList<MonitorSettings*> settings;
    foreach(MonitorInfo *monitorInfo, timeoutSettings) {
      settings.append((MonitorSettings*)monitorInfo);
    }
    backend->setMonitorsSettings(settings);
    deleteTimeoutData();
  }
  else
    timeoutDialog->setValue(time);
}

QList<MonitorSettings*> MonitorSettingsDialog::getMonitorsSettings() {
  // Build list of monitor and their settings
  QList<MonitorSettings*> settings;
  foreach(Monitor *monitor, monitors) {
    MonitorSettings *s = new MonitorSettings();
    settings.append(s);
    s->name = monitor->name;
    s->enabledOk = monitor->enable->isChecked();
    s->currentMode = monitor->resolutionCombo->currentText();
    s->currentRate = monitor->rateCombo->currentText();
    s->position = (MonitorSettings::Position)monitor->positionCombo->currentData().toInt();
    s->positionRelativeToOutput = monitor->relativeToOutputCombo->currentText();
    if(ui.primaryCombo->currentText()==monitor->name)
      s->primaryOk = true;
  }
  if(ui.unify->isChecked()) {
    foreach(MonitorSettings *s, settings) {
      s->position = MonitorSettings::None;
    }
  }
  return settings;
}

void MonitorSettingsDialog::setMonitorsConfig() {
  deleteTimeoutData();
  timeoutSettings = backend->getMonitorsInfo();
  // Show timeout dialog
  timeoutDialog = new QProgressDialog(tr("OK?"), tr("Yes"), 0, 10);
  connect(timeoutDialog, SIGNAL(canceled()), this, SLOT(onCancelSettings()));
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
  timer->start(1000);
  // Build list of monitor and their settings
  QList<MonitorSettings*> settings = getMonitorsSettings();
  backend->setMonitorsSettings(settings);
  foreach(MonitorSettings *s, settings) {
    delete s;
  }
}

void MonitorSettingsDialog::chooseMaxResolution(Monitor* monitor) {
  if(monitor->resolutionCombo->count() > 1)
    monitor->resolutionCombo->setCurrentIndex(1);
}

// turn on both laptop LCD and the external monitor
void MonitorSettingsDialog::onUseBoth() {
  if(monitors.length() ==0)
    return;
  ui.unify->setChecked(true);
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

void MonitorSettingsDialog::onExtended() {
  ui.unify->setChecked(false);
  int i = 0;
  foreach(Monitor *monitor, monitors) {
    chooseMaxResolution(monitor);
    monitor->enable->setChecked(true);
    if(i==0) {
      monitor->positionCombo->setCurrentIndex(0);
    } else {
      monitor->positionCombo->setCurrentIndex(1);
      monitor->relativeToOutputCombo->setCurrentIndex(i);
    }
    i++;
  }
  accept();
}

void MonitorSettingsDialog::setupUi() {
  ui.setupUi(this);
  connect(ui.useBoth, SIGNAL(clicked(bool)), SLOT(onUseBoth()));
  connect(ui.externalOnly, SIGNAL(clicked(bool)), SLOT(onExternalOnly()));
  connect(ui.laptopOnly, SIGNAL(clicked(bool)), SLOT(onLaptopOnly()));
  connect(ui.extended, SIGNAL(clicked(bool)), SLOT(onExtended()));
  connect(ui.unify, SIGNAL(stateChanged(int)), SLOT(onUnifyChanged(int)));

  connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onDialogButtonClicked(QAbstractButton*)));
  aboutButton = new QPushButton(ui.buttonBox);
  aboutButton->setText(tr("About"));
  ui.buttonBox->addButton(aboutButton, QDialogButtonBox::HelpRole);

  // Get monitors information
  QList<MonitorInfo*> monitorsInfo = backend->getMonitorsInfo();

  if(monitorsInfo.length() == 1)
    ui.unify->setEnabled(false);

  QStringList outputs;
  outputs.append("");
  foreach(MonitorInfo *monitorInfo, monitorsInfo) {
    outputs.append(monitorInfo->name);
  }
  int i = 0;
  foreach(MonitorInfo *monitorInfo, monitorsInfo) {
    Monitor *monitor = new Monitor();
    monitor->setParent(this);
    monitors.append(monitor);
    monitor->name = monitorInfo->name;
    monitor->modes = monitorInfo->modes;
     if(! LVDS && ( monitor->name.startsWith("LVDS") || monitor->name.startsWith("PANEL") )  ) {
        LVDS = monitor;
    }
    QGroupBox* box = new QGroupBox(this);
    QString title = QString("Monitor %1: %2 (%3)")
        .arg(i + 1)
        .arg(monitorInfo->name)
        .arg(humanReadableName(monitor));
    qDebug() << "Monitor" << title;
    box->setTitle(title);
    Ui::MonitorWidget mui = Ui::MonitorWidget();
    mui.setupUi(box);
    ui.monitorLayout->insertWidget(ui.monitorLayout->count() - 1, box);
    ui.monitorLayout->setStretchFactor(box, 0);

    monitor->enable = mui.enabled;
    monitor->resolutionCombo = mui.resolution;
    mui.resolution->setProperty("modeLines", qVariantFromValue<QHash<QString, QStringList>   >(monitorInfo->modeLines));
    mui.resolution->setProperty("rateCombo", qVariantFromValue<QComboBox*>(mui.rate));
    monitor->rateCombo = mui.rate;
    monitor->positionCombo = mui.positionCombo;
    monitor->relativeToOutputCombo = mui.relativeToOutputCombo;
    monitor->positionLabel = mui.positionLabel;
    mui.positionCombo->addItem(tr(""), qVariantFromValue<int>(MonitorSettings::None));
    mui.positionCombo->addItem(tr("Left of"), qVariantFromValue<int>(MonitorSettings::Left));
    mui.positionCombo->addItem(tr("Right of"), qVariantFromValue<int>(MonitorSettings::Right));
    mui.positionCombo->addItem(tr("Above of"), qVariantFromValue<int>(MonitorSettings::Above));
    mui.positionCombo->addItem(tr("Below of"), qVariantFromValue<int>(MonitorSettings::Bellow));
    QStringList _outputs = outputs;
    _outputs.removeOne(monitor->name);
    mui.relativeToOutputCombo->addItems(_outputs);
    if(monitorsInfo.length() == 1) {
      mui.positionCombo->setEnabled(false);
      mui.relativeToOutputCombo->setEnabled(false);
      mui.positionLabel->setEnabled(false);
    }

    // turn off screen is not allowed since there should be at least one monitor available.
    if(monitorsInfo.length() == 1)
      monitor->enable->setEnabled(false);

    if(monitorInfo->enabledOk)
      monitor->enable->setChecked(true);

    connect(monitor->resolutionCombo, SIGNAL(currentIndexChanged(int)), SLOT(onResolutionChanged(int)));
    monitor->resolutionCombo->addItem(tr("Auto"));

    foreach(QString _mode_line, monitor->modes) {
      monitor->resolutionCombo->addItem(_mode_line);
    }
    monitor->resolutionCombo->setCurrentIndex( monitor->resolutionCombo->findText(monitorInfo->currentMode) );
    monitor->rateCombo->setCurrentIndex( monitor->rateCombo->findText(monitorInfo->currentRate) );
    delete monitorInfo;
    ++i;
  }
  
  // If this is a laptop and there is an external monitor, offer quick options
  if(monitors.length() == 2) {
    ui.tabWidget->setCurrentIndex(0);
    // If there is only two monitors,offer quick options
    if(! LVDS) {
      ui.useBoth->setText(tr("Show the same screen on both"));
      ui.externalOnly->setText(tr("Turn off first monitor and use second monitor only"));
      ui.laptopOnly->setText(tr("Turn off second monitor and use first monitor only"));
      LVDS = monitors[0];
    }
  } else {
    ui.tabWidget->removeTab(0);
  }
  // Add outputs to primary ComboBox
  foreach(QString output, outputs) {
    ui.primaryCombo->addItem(output);
  }
  onUnifyChanged(1);
}

void MonitorSettingsDialog::accept() {
  setMonitorsConfig();
  QDialog::accept();
}


void MonitorSettingsDialog::onDialogButtonClicked(QAbstractButton* button) {
  if(ui.buttonBox->standardButton(button) == QDialogButtonBox::Apply) {
    setMonitorsConfig();
   } else if(ui.buttonBox->standardButton(button) == QDialogButtonBox::Save) {
     // Save config and exit
     QList<MonitorSettings*> settings = getMonitorsSettings();
     QString cmd = backend->getCommand(settings);
     foreach(MonitorSettings *s, settings) {
       delete s;
     }
     QString desktop = QString("[Desktop Entry]\n"
       "Type=Application\n"
       "Name=LXQt-config-monitor autostart\n"
       "Comment=Autostart monitor settings for LXQt-config-monitor\n"
       "Exec=%1\n"
       "OnlyShowIn=LXQt\n").arg(cmd);
     QFile file(QDir::homePath() + "/.config/autostart/lxqt-config-monitor-autostart.desktop");
     if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
     QTextStream out(&file);
     out << desktop;
     out.flush();
     file.close();
     QDialog::accept();
   } else if(button == aboutButton) {
    // about dialog
    QMessageBox::about(this, tr("About"), tr("LXQt-config-monitor\n\nMonitor configuration tool for LXQt."));
  }
}

