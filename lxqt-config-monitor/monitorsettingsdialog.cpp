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
#include <QTimer>
#include <QProgressBar>
#include <QDebug>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>

#include "monitorwidget.h"
#include "timeoutdialog.h"
#include "xrandr.h"


MonitorSettingsDialog::MonitorSettingsDialog(MonitorSettingsBackend* backend):
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


void MonitorSettingsDialog::deleteTimeoutData() {
  timeoutDialog = NULL;
  Q_FOREACH(MonitorInfo * monitorInfo, timeoutSettings) {
    delete monitorInfo;
  }
  timeoutSettings.clear();
}

void MonitorSettingsDialog::onCancelSettings() {
  // restore the old settings
  QList<MonitorSettings*> settings;
  Q_FOREACH(MonitorInfo * monitorInfo, timeoutSettings) {
    settings.append((MonitorSettings*)monitorInfo);
  }
  backend->setMonitorsSettings(settings);
  deleteTimeoutData();
}

QList<MonitorSettings*> MonitorSettingsDialog::getMonitorsSettings() {
  // Build list of monitor and their settings
  QList<MonitorSettings*> settings;
  Q_FOREACH(MonitorWidget * monitor, monitors) {
    MonitorSettings* s = monitor->getSettings();
    settings.append(s);
    if(ui.primaryCombo->currentText() == monitor->monitorInfo->name)
      s->primaryOk = true;
  }
  if(ui.unify->isChecked()) {
    Q_FOREACH(MonitorSettings * s, settings) {
      s->position = MonitorSettings::None;
    }
  }
  return settings;
}

void MonitorSettingsDialog::setMonitorsConfig() {
  deleteTimeoutData();
  timeoutSettings = backend->getMonitorsInfo();
  // Show timeout dialog
  timeoutDialog = new TimeoutDialog(this);
  connect(timeoutDialog, SIGNAL(rejected()), this, SLOT(onCancelSettings()));
  connect(timeoutDialog, SIGNAL(finished(int)), timeoutDialog, SLOT(deleteLater()));
  // Build list of monitor and their settings
  QList<MonitorSettings*> settings = getMonitorsSettings();
  backend->setMonitorsSettings(settings);
  Q_FOREACH(MonitorSettings * s, settings) {
    delete s;
  }
  timeoutDialog->show();
}

// turn on both laptop LCD and the external monitor
void MonitorSettingsDialog::onUseBoth() {
  if(monitors.length() == 0)
    return;
  ui.unify->setChecked(true);
  MonitorWidget* monitor = monitors[0];
  bool ok;
  QString mode;
  for(int i = 0; i < monitor->monitorInfo->modes.length(); i++) {
    mode = monitor->monitorInfo->modes[i];
    ok = true;
    Q_FOREACH(MonitorWidget * monitor2, monitors) {
      ok = ok && monitor2->monitorInfo->modes.contains(mode);
    }
    if(ok)
      break;
  }
  qDebug() << "Mode selected" << mode << ok;
  Q_FOREACH(MonitorWidget * monitor2, monitors) {
    int index = monitor2->monitorInfo->modes.indexOf(mode) + 1;
    if(monitor2->ui.resolutionCombo->count() > index)
      monitor2->ui.resolutionCombo->setCurrentIndex(index);
    else
      monitor2->chooseMaxResolution();
    monitor2->enableMonitor(true);
    qDebug() << "Mode selected index" << index << "Mode" << monitor->ui.resolutionCombo->currentText();
  }
  setMonitorsConfig();
}

// external monitor only
void MonitorSettingsDialog::onExternalOnly() {
  Q_FOREACH(MonitorWidget * monitor, monitors) {
    monitor->chooseMaxResolution();
    monitor->enableMonitor(monitor != LVDS);
  }
  setMonitorsConfig();
}

// laptop panel - LVDS only
void MonitorSettingsDialog::onLaptopOnly() {
  Q_FOREACH(MonitorWidget * monitor, monitors) {
    monitor->chooseMaxResolution();
    monitor->enableMonitor(monitor == LVDS);
  }
  setMonitorsConfig();
}

void MonitorSettingsDialog::onExtended() {
  ui.unify->setChecked(false);
  int i = 0;
  Q_FOREACH(MonitorWidget * monitor, monitors) {
    monitor->chooseMaxResolution();
    monitor->enableMonitor(true);
    if(i == 0) {
      monitor->ui.positionCombo->setCurrentIndex(0);
    }
    else {
      monitor->ui.positionCombo->setCurrentIndex(1);
      monitor->ui.relativeToOutputCombo->setCurrentIndex(i);
    }
    i++;
  }
  setMonitorsConfig();
}

void MonitorSettingsDialog::setupUi() {
  ui.setupUi(this);
  connect(ui.useBoth, SIGNAL(clicked(bool)), SLOT(onUseBoth()));
  connect(ui.externalOnly, SIGNAL(clicked(bool)), SLOT(onExternalOnly()));
  connect(ui.laptopOnly, SIGNAL(clicked(bool)), SLOT(onLaptopOnly()));
  connect(ui.extended, SIGNAL(clicked(bool)), SLOT(onExtended()));
  connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onDialogButtonClicked(QAbstractButton*)));

  // Get monitors information
  QList<MonitorInfo*> monitorsInfo = backend->getMonitorsInfo();

  // Search if LVSD monitor is connected
  Q_FOREACH(MonitorInfo * monitorInfo, monitorsInfo) {
    if(! LVDS && (monitorInfo->name.startsWith("LVDS") || monitorInfo->name.startsWith("PANEL"))) {
      MonitorInfo::LVDS_Ok = true;
      break;
    }
  }

  int i = 0;
  Q_FOREACH(MonitorInfo * monitorInfo, monitorsInfo) {
    ui.primaryCombo->addItem(monitorInfo->name);
    if(monitorInfo->primaryOk)
      ui.primaryCombo->setCurrentIndex(ui.primaryCombo->findText(monitorInfo->name));

    qDebug() << "Monitor" << monitorInfo->name;
    MonitorWidget* monitor = new MonitorWidget(monitorInfo, monitorsInfo, this);
    QString title = QString("Monitor %1: %2 (%3) %4")
                    .arg(i + 1)
                    .arg(monitor->monitorInfo->name)
                    .arg(monitor->monitorInfo->humanReadableName())
                    .arg(monitor->monitorInfo->vendor);
    qDebug() << "Monitor" << title;
    monitor->setTitle(title);

    connect(ui.unify, SIGNAL(toggled(bool)), monitor, SLOT(disablePositionOption(bool)));
    monitors.append(monitor);
    if(! LVDS && (monitorInfo->name.startsWith("LVDS") || monitorInfo->name.startsWith("PANEL"))) {
      LVDS = monitor;
    }
    ui.stackedWidget->addWidget(monitor);
    ui.monitorList->addItem(monitor->monitorInfo->name);
    ++i;
  }
  ui.monitorList->setCurrentRow(0);
  // set the max width of the list widget to the maximal width of its rows + the width of a vertical scrollbar.
  ui.monitorList->setMaximumWidth(ui.monitorList->sizeHintForColumn(0) + style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 40);

  // are the monitors unified?
  if(monitorsInfo.length() > 1)
    ui.unify->setChecked(backend->isUnified(monitorsInfo));
  else // disable the option if we only have one monitor
    ui.unify->setEnabled(false);

  // If this is a laptop and there is an external monitor, offer quick options
  if(monitors.length() == 2) {
    ui.tabWidget->setCurrentIndex(0);
    // If there is only two monitors,offer quick options
    if(! LVDS) {
      LVDS = monitors[0];
    }
  }
  else {
    ui.tabWidget->removeTab(0);
  }
  
  // Sets position tab
  int monitorsWidth = 0.0;
  int monitorsHeight = 0.0;
  QGraphicsScene *scene = new QGraphicsScene();
  Q_FOREACH(MonitorInfo * monitorInfo, monitorsInfo) {
    QGraphicsTextItem *textItem = scene->addText(monitorInfo->name);
    textItem->setPos(0,0);
    QGraphicsRectItem *rectItem = scene->addRect(0,0,800,480);
    rectItem->setAcceptedMouseButtons(Qt::LeftButton);
    rectItem->setFlags(QGraphicsItem::ItemIsMovable);
    textItem->setParentItem(rectItem);
    qreal fontWidth = QFontMetrics(textItem->font()).width(monitorInfo->name+"  "); 
    textItem->setScale((qreal)rectItem->rect().width()/fontWidth);
    monitorsWidth+=rectItem->rect().width();
  }
  ui.positionGraphicsView->scale(1.0/8.0,1.0/8.0);
  ui.positionGraphicsView->setScene(scene);

  adjustSize();
}

void MonitorSettingsDialog::accept() {
  setMonitorsConfig();
  QDialog::accept();
}


void MonitorSettingsDialog::onDialogButtonClicked(QAbstractButton* button) {
  if(ui.buttonBox->standardButton(button) == QDialogButtonBox::Apply) {
    setMonitorsConfig();
  }
  else if(ui.buttonBox->standardButton(button) == QDialogButtonBox::Save) {
    // Save config and exit
    QList<MonitorSettings*> settings = getMonitorsSettings();
    QString cmd = backend->getCommand(settings);
    Q_FOREACH(MonitorSettings * s, settings) {
      delete s;
    }
    QString desktop = QString("[Desktop Entry]\n"
                              "Type=Application\n"
                              "Name=LXQt-config-monitor autostart\n"
                              "Comment=Autostart monitor settings for LXQt-config-monitor\n"
                              "Exec=%1\n"
                              "OnlyShowIn=LXQt\n").arg(cmd);
    QFile file(QDir::homePath() + "/.config/autostart/lxqt-config-monitor-autostart.desktop");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    QTextStream out(&file);
    out << desktop;
    out.flush();
    file.close();
    QDialog::accept();
  }
}

