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


#ifndef KEYBOARDCONFIG_H
#define KEYBOARDCONFIG_H

#include <QDialog>
#include "ui_keyboardconfig.h"

namespace LxQt {
  class Settings;
}
class QSettings;

class KeyboardConfig : public QWidget {
  Q_OBJECT

public:
  KeyboardConfig(LxQt::Settings* _settings, QSettings* _qtSettings, QWidget* parent = 0);
  virtual ~KeyboardConfig();

  void accept();

public Q_SLOTS:
  void reset();

private:
  void setLeftHandedMouse();
  void loadSettings();
  void initControls();

private Q_SLOTS:
  void onKeyboardSliderChanged(int value);
  void onKeyboardBeepToggled(bool checked);
  void onCorsorFlashTimeChanged(int value);

private:
  Ui::KeyboardConfig ui;
  LxQt::Settings* settings;
  QSettings* qtSettings;
  int delay;
  int oldDelay;
  int interval;
  int oldInterval;
  bool beep;
  bool oldBeep;
};

#endif // KEYBOARDCONFIG_H
