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


#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <QStringList>
#include <QHash>
#include <QList>
#include <QRect>

//Settings to pass to backend
class MonitorSettings: public QObject {
  Q_OBJECT
public:
  MonitorSettings(QObject* parent = 0);
  QString name;
  QString currentMode;
  QString currentRate;
  QString gamma;
  QString brightness;
  int xPos; // not used yet
  int yPos; // not used yet
  bool enabledOk;
  enum Position {None = 0, Left, Right, Above, Bellow};
  Position position;
  QString positionRelativeToOutput;
  bool primaryOk;

  QSize currentSize();
  QRect geometry();
};


// Monitor information from backend
class MonitorInfo: public MonitorSettings {
  Q_OBJECT
public:
  MonitorInfo(QObject* parent = 0);
  QStringList modes; // Modes of this monitor in order
  QHash<QString, QStringList> modeLines; // Rates suported by each mode
  QString preferredMode;
  QString preferredRate;
  QString edid; // EDID data, not used yet, can be used to detect vendor name of the monitor
  
  QString humanReadableName();
};


class MonitorSettingsBackend: public QObject {
  Q_OBJECT
public:
  virtual QList<MonitorInfo*> getMonitorsInfo() = 0;
  virtual bool setMonitorsSettings(const QList<MonitorSettings*> monitors) = 0;
  virtual QString getCommand(const QList<MonitorSettings*> monitors) = 0;
  virtual bool isUnified(const QList<MonitorInfo*> monitors);
};

#endif // _MONITOR_H_