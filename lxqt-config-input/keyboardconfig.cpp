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
#include <LXQt/Settings>
#include <QDir>
#include <QFile>
#include <QStringBuilder>

// FIXME: how to support XCB or Wayland?
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

KeyboardConfig::KeyboardConfig(LXQt::Settings* _settings, QSettings* _qtSettings, QWidget* parent):
  QWidget(parent),
  settings(_settings),
  qtSettings(_qtSettings),
  delay(500),
  oldDelay(500),
  interval(30),
  oldInterval(30),
  flashTime(1000),
  oldFlashTime(1000),
  beep(true),
  oldBeep(true),
  numlock(false),
  oldNumlock(false) {

  ui.setupUi(this);

  /* read the config file */
  loadSettings();
  initControls();

  connect(ui.keyboardDelay, &QAbstractSlider::valueChanged, ui.label_delay, QOverload<int>::of(&QLabel::setNum));
  connect(ui.keyboardInterval, &QAbstractSlider::valueChanged, ui.label_interval, QOverload<int>::of(&QLabel::setNum));

  // set_range_stops(ui.keyboardDelay, 10);
  connect(ui.keyboardDelay, &QAbstractSlider::valueChanged, this, &KeyboardConfig::settingsChanged);
  // set_range_stops(ui.keyboardInterval, 10);
  connect(ui.keyboardInterval, &QAbstractSlider::valueChanged, this, &KeyboardConfig::settingsChanged);
  connect(ui.keyboardBeep, &QAbstractButton::clicked, this, &KeyboardConfig::settingsChanged);
  connect(ui.cursorFlashTime, QOverload<int>::of(&QSpinBox::valueChanged), this, &KeyboardConfig::settingsChanged);
  connect(ui.keyboardNumLock, &QAbstractButton::clicked, this, &KeyboardConfig::settingsChanged);

  if (QGuiApplication::platformName() == QLatin1String("wayland"))
  { // disable the settings that don't work under Wayland
    ui.keyboardDelay->setEnabled(false);
    ui.keyboardInterval->setEnabled(false);
    ui.keyboardBeep->setEnabled(false);
    ui.keyboardNumLock->setEnabled(false);
  }
}

KeyboardConfig::~KeyboardConfig() {

}

void KeyboardConfig::initControls() {
  ui.keyboardDelay->blockSignals(true);
  ui.keyboardDelay->setValue(delay);
  ui.label_delay->setNum(delay);
  ui.keyboardDelay->blockSignals(false);

  ui.keyboardInterval->blockSignals(true);
  ui.keyboardInterval->setValue(interval);
  ui.label_interval->setNum(interval);
  ui.keyboardInterval->blockSignals(false);

  ui.keyboardBeep->setChecked(beep);
  ui.keyboardNumLock->setChecked(numlock);

  ui.cursorFlashTime->blockSignals(true);
  ui.cursorFlashTime->setValue(flashTime);
  ui.cursorFlashTime->blockSignals(false);
}

void KeyboardConfig::applyConfig()
{
  bool acceptSetting = false;

  Display* dpy = nullptr;
  if (auto x11NativeInterface = qGuiApp->nativeInterface<QNativeInterface::QX11Application>())
    dpy = x11NativeInterface->display();

  /* apply keyboard values */
  if(delay != ui.keyboardDelay->value() || interval != ui.keyboardInterval->value())
  {
    delay = ui.keyboardDelay->value();
    interval = ui.keyboardInterval->value();
    if (dpy)
      XkbSetAutoRepeatRate(dpy, XkbUseCoreKbd, delay, interval);
    acceptSetting = true;
  }

  if(beep != ui.keyboardBeep->isChecked())
  {
    beep = ui.keyboardBeep->isChecked();
    if (dpy)
    {
      XKeyboardControl values;
      values.bell_percent = beep ? -1 : 0;
      XChangeKeyboardControl(dpy, KBBellPercent, &values);
    }
    acceptSetting = true;
  }

  if(flashTime != ui.cursorFlashTime->value())
  {
    flashTime = ui.cursorFlashTime->value();
    acceptSetting = true;
  }

  if(numlock != ui.keyboardNumLock->isChecked())
  {
    numlock = ui.keyboardNumLock->isChecked();
    acceptSetting = true;
  }

  if(acceptSetting)
    accept();
}

void KeyboardConfig::loadSettings() {
  settings->beginGroup(QStringLiteral("Keyboard"));
  oldDelay = delay = settings->value(QStringLiteral("delay"), 500).toInt();
  oldInterval = interval = settings->value(QStringLiteral("interval"), 30).toInt();
  oldBeep = beep = settings->value(QStringLiteral("beep"), true).toBool();
  oldNumlock = numlock = settings->value(QStringLiteral("numlock"), false).toBool();
  settings->endGroup();

  qtSettings->beginGroup(QLatin1String("Qt"));
  oldFlashTime = flashTime = qtSettings->value(QLatin1String("cursorFlashTime"), 1000).toInt();
  qtSettings->endGroup();
}

void KeyboardConfig::accept() {
  settings->beginGroup(QStringLiteral("Keyboard"));
  settings->setValue(QStringLiteral("delay"), delay);
  settings->setValue(QStringLiteral("interval"), interval);
  settings->setValue(QStringLiteral("beep"), beep);
  settings->setValue(QStringLiteral("numlock"), numlock);
  settings->endGroup();

  qtSettings->beginGroup(QLatin1String("Qt"));
  qtSettings->setValue(QLatin1String("cursorFlashTime"), flashTime);
  qtSettings->endGroup();
}

void KeyboardConfig::reset() {
  /* restore to original settings */
  /* keyboard */
  delay = oldDelay;
  interval = oldInterval;
  beep = oldBeep;
  numlock = oldNumlock;
  flashTime = oldFlashTime;
  if(auto x11NativeInterface = qGuiApp->nativeInterface<QNativeInterface::QX11Application>()) {
    if(Display* dpy = x11NativeInterface->display()) {
      XkbSetAutoRepeatRate(dpy, XkbUseCoreKbd, delay, interval);
    }
  }
  /* FIXME: beep? */

  initControls();
  accept();
}
