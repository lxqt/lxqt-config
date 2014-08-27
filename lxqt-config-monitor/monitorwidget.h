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


#ifndef _MONITORWIDGET_H_
#define _MONITORWIDGET_H_

#include <QGroupBox>
#include <QStringList>
#include <QHash>
#include <QList>
#include "ui_monitorwidget.h"

class MonitorInfo;
class MonitorSettings;

// Monitor info
class MonitorWidget : public QGroupBox {
  Q_OBJECT

public:
  MonitorWidget(MonitorInfo* monitor, const QList< MonitorInfo* > monitorsInfo, QWidget* parent = 0);
  MonitorSettings* getSettings();
  void chooseMaxResolution();
  void enableMonitor(bool enable);

  MonitorInfo* monitorInfo;

  Ui::MonitorWidget ui;
public Q_SLOTS:
  void disablePositionOption(bool disabled);

private slots:
  void onResolutionChanged(int);
};

#endif // _MONITORWIDGET_H_