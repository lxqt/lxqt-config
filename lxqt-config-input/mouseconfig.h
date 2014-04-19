/*
    Copyright (C) 2013-2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

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


#ifndef MOUSECONFIG_H
#define MOUSECONFIG_H

#include <QWidget>
#include "ui_mouseconfig.h"

namespace LxQt {
  class Settings;
}
class QSettings;

class MouseConfig : public QWidget {
  Q_OBJECT

public:
  MouseConfig(LxQt::Settings* _settings, QSettings* _qtSettings, QWidget* parent);
  virtual ~MouseConfig();

  void accept();
public Q_SLOTS:
  void reset();

private:
  void setLeftHandedMouse();
  void loadSettings();
  void initControls();

private Q_SLOTS:
  void onMouseAccelChanged(int value);
  void onMouseThresholdChanged(int value);
  void onMouseLeftHandedToggled(bool checked);
  void onDoubleClickIntervalChanged(int value);
  void onWheelScrollLinesChanged(int value);

private:
  Ui::MouseConfig ui;
  LxQt::Settings* settings;
  QSettings* qtSettings;
  int accel;
  int oldAccel;
  int threshold;
  int oldThreshold;
  bool leftHanded;
  bool oldLeftHanded;
};

#endif // MOUSECONFIG_H
