/*
    Copyright (C) 2014  P.L. Lucas <selairi@gmail.com>

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

#include "monitor.h"
#include <QDebug>

bool MonitorInfo::LVDS_Ok = false;

QSize sizeFromString(QString str) {
  int width = 0;
  int height = 0;
  int x = str.indexOf('x');
  if(x > 0) {
    width = str.left(x).toInt();
    height = str.mid(x + 1).toInt();
  }
  return QSize(width, height);
}

MonitorSettings::MonitorSettings(QObject* parent): QObject(parent) {
  position = None;
  primaryOk = false;
  enabledOk = false;
}

MonitorInfo::MonitorInfo(QObject* parent): MonitorSettings(parent) {
}

QString MonitorInfo::humanReadableName() {
  if(name.startsWith("LVDS"))
    return tr("Laptop LCD Monitor");
  else if(name.startsWith("VGA") || name.startsWith("Analog"))
    return LVDS_Ok ? tr("External VGA Monitor") :  tr("VGA Monitor");
  else if(name.startsWith("DVI") || name.startsWith("TMDS") || name.startsWith("Digital") || name.startsWith("LVDS"))
    return LVDS_Ok ? tr("External DVI Monitor") : tr("DVI Monitor");
  else if(name.startsWith("TV") || name.startsWith("S-Video"))
    return tr("TV");
  else if(name == "default")
    return tr("Default Monitor");
  return name;
}


MonitorMode::MonitorMode(QString modeName, QObject *parent):QObject(parent) {
  mode = modeName;
  width = -1;
  height = -1;
}

QSize sizeFromString(QString str) {
  int width = 0;
  int height = 0;
  int x = str.indexOf('x');
  if(x > 0) {
    width = str.left(x).toInt();
    height = str.mid(x + 1).toInt();
  }
  return QSize(width, height);
}
