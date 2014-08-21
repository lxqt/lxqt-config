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

#include <QComboBox>
#include <QCheckBox>
#include <QStringList>
#include <QHash>
#include <QList>
#include <QLabel>

// Monitor info
class Monitor : public QObject {
Q_OBJECT

public:
  Monitor(QObject *parent = 0);
  QString name;
  QStringList modes; // Suported modes in order

  QCheckBox* enable;
  QComboBox* resolutionCombo;
  QComboBox* rateCombo;
  QComboBox* positionCombo;
  QComboBox* relativeToOutputCombo;
  QLabel* positionLabel;
};


//Settings to pass to backend
class MonitorSettings: public QObject {
	Q_OBJECT
public:
  MonitorSettings(QObject *parent = 0);
  QString name;
  QString currentMode;
  QString currentRate;
  bool enabledOk; 
  enum Position {None, Left, Right, Above, Bellow};  
  Position position;
  QString positionRelativeToOutput;
  bool primaryOk;
};


// Monitor information from backend
class MonitorInfo: public MonitorSettings {
	Q_OBJECT
public:
  MonitorInfo(QObject *parent = 0);
  QStringList modes; // Modes of this monitor in order
  QHash<QString, QStringList> modeLines; // Rates suported by each mode
  QString preferredMode;
  QString preferredRate;
};


class Backend: public QObject {
	Q_OBJECT
public:
  virtual QList<MonitorInfo*> getMonitorsInfo() = 0;
  virtual bool setMonitorsSettings(const QList<MonitorSettings*> monitors) = 0;
  virtual QString getCommand(const QList<MonitorSettings*> monitors) = 0;
};

#endif // _MONITOR_H_