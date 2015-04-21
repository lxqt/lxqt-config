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

#include <QJsonArray>
#include <QJsonObject>
#include "applydialog.h"
#include "configure.h"
#include <QDebug>
#include <QJsonDocument>


ApplyDialog::ApplyDialog(LxQt::Settings*applicationSettings, QWidget* parent):
  QDialog(parent) {
  
  this->applicationSettings = applicationSettings;

  ui.setupUi(this);
  
  ui.apply->setIcon(QIcon::fromTheme("system-run"));
  ui.save->setIcon(QIcon::fromTheme("document-save"));

  
  QSize size(128,64);
  ui.apply->setIconSize(size);
  ui.save->setIconSize(size);
  
  loadSettings();
}

void ApplyDialog::setHardwareIdentifier(QString hardwareIdentifier) {
	this->hardwareIdentifier = hardwareIdentifier;
	loadSettings();
}

void ApplyDialog::loadSettings() {
  ui.allConfigs->clear();
  ui.hardwareCompatibleConfigs->clear();
  applicationSettings->beginGroup("configMonitor");
  QJsonArray  savedConfigs = QJsonDocument::fromJson(applicationSettings->value("saved").toByteArray()).array();
  foreach (const QJsonValue & v, savedConfigs) {
    QJsonObject o = v.toObject();
    QListWidgetItem *item = new QListWidgetItem(o["name"].toString(), ui.allConfigs);
    item->setData(Qt::UserRole, QVariant(o));
    if(o["hardwareIdentifier"].toString() == hardwareIdentifier) {
      QListWidgetItem *item = new QListWidgetItem(o["name"].toString(), ui.hardwareCompatibleConfigs);
      item->setData(Qt::UserRole, QVariant(o));
    }
  }
  applicationSettings->endGroup();
}