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


#include "keyboardconfig.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <lxqt/LxQtSettings>
#include <QDir>
#include <QFile>
#include <QStringBuilder>

// FIXME: how to support XCB or Wayland?
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#ifdef Q_WS_X11
extern void qt_x11_apply_settings_in_all_apps();
#endif

KeyboardConfig::KeyboardConfig(LxQt::Settings* _settings, QSettings* _qtSettings, QWidget* parent):
  QWidget(parent),
  settings(_settings),
  qtSettings(_qtSettings),
  delay(500),
  oldDelay(500),
  interval(30),
  oldInterval(30),
  beep(true),
  oldBeep(true) {

  ui.setupUi(this);

  /* read the config flie */
  loadSettings();
  initControls();

  // set_range_stops(ui.keyboardDelay, 10);
  connect(ui.keyboardDelay, SIGNAL(valueChanged(int)), SLOT(onKeyboardSliderChanged(int)));
  // set_range_stops(ui.keyboardInterval, 10);
  connect(ui.keyboardInterval, SIGNAL(valueChanged(int)), SLOT(onKeyboardSliderChanged(int)));
  connect(ui.keyboardBeep, SIGNAL(toggled(bool)), SLOT(onKeyboardBeepToggled(bool)));
  connect(ui.cursorFlashTime, SIGNAL(valueChanged(int)), SLOT(onCorsorFlashTimeChanged(int)));
}

KeyboardConfig::~KeyboardConfig() {

}

void KeyboardConfig::initControls() {
  ui.keyboardDelay->setValue(delay);
  ui.keyboardInterval->setValue(interval);
  ui.keyboardBeep->setChecked(beep);

  qtSettings->beginGroup(QLatin1String("Qt"));
  int value = qtSettings->value(QLatin1String("cursorFlashTime"), 1000).toInt();
  ui.cursorFlashTime->setValue(value);
  qtSettings->endGroup();
}

void KeyboardConfig::onKeyboardSliderChanged(int value) {
  QSlider* slider = static_cast<QSlider*>(sender());

  if(slider == ui.keyboardDelay)
    delay = value;
  else if(slider == ui.keyboardInterval)
    interval = value;

  /* apply keyboard values */
  XkbSetAutoRepeatRate(QX11Info::display(), XkbUseCoreKbd, delay, interval);
  
  accept();
}

void KeyboardConfig::onKeyboardBeepToggled(bool checked) {
  XKeyboardControl values;
  beep = checked;
  values.bell_percent = beep ? -1 : 0;
  XChangeKeyboardControl(QX11Info::display(), KBBellPercent, &values);

  accept();
}

void KeyboardConfig::onCorsorFlashTimeChanged(int value)
{
  qtSettings->beginGroup(QLatin1String("Qt"));
  qtSettings->setValue(QLatin1String("cursorFlashTime"), value);
  qtSettings->endGroup();
  qtSettings->sync();
#ifdef Q_WS_X11
  qt_x11_apply_settings_in_all_apps();
#endif
}


void KeyboardConfig::loadSettings() {
  settings->beginGroup("Keyboard");
  oldDelay = delay = settings->value("delay", 500).toInt();
  oldInterval = interval = settings->value("interval", 30).toInt();
  oldBeep = beep = settings->value("beep", true).toBool();
  settings->endGroup();
}

void KeyboardConfig::accept() {
  settings->beginGroup("Keyboard");
  settings->setValue("delay", delay);
  settings->setValue("interval", interval);
  settings->setValue("beep", beep);
  settings->endGroup();
}

void KeyboardConfig::reset() {
  /* restore to original settings */
  /* keyboard */
  delay = oldDelay;
  interval = oldInterval;
  beep = oldBeep;
  XkbSetAutoRepeatRate(QX11Info::display(), XkbUseCoreKbd, delay, interval);
  /* FIXME: beep? */

  initControls();
  accept();
}
