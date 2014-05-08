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


#include "mouseconfig.h"
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

MouseConfig::MouseConfig(LxQt::Settings* _settings, QSettings* _qtSettings, QWidget* parent):
  QWidget(parent),
  settings(_settings),
  qtSettings(_qtSettings),
  accel(20),
  oldAccel(20),
  threshold(10),
  oldThreshold(10),
  leftHanded(false),
  oldLeftHanded(false) {

  ui.setupUi(this);

  /* read the config flie */
  loadSettings();
  initControls();

  // set_range_stops(ui.mouseAccel, 10);
  connect(ui.mouseAccel, SIGNAL(valueChanged(int)), SLOT(onMouseAccelChanged(int)));
  // set_range_stops(ui.mouseThreshold, 10);
  connect(ui.mouseThreshold, SIGNAL(valueChanged(int)), SLOT(onMouseThresholdChanged(int)));
  connect(ui.mouseLeftHanded, SIGNAL(toggled(bool)), SLOT(onMouseLeftHandedToggled(bool)));
  
  connect(ui.doubleClickInterval, SIGNAL(valueChanged(int)), SLOT(onDoubleClickIntervalChanged(int)));
  connect(ui.wheelScrollLines, SIGNAL(valueChanged(int)), SLOT(onWheelScrollLinesChanged(int)));
}

MouseConfig::~MouseConfig() {
}

void MouseConfig::initControls() {
  ui.mouseAccel->setValue(accel);
  ui.mouseThreshold->setValue(110 - threshold);
  ui.mouseLeftHanded->setChecked(leftHanded);
  
  qtSettings->beginGroup(QLatin1String("Qt"));
  int value = qtSettings->value(QLatin1String("doubleClickInterval"), 400).toInt();
  ui.doubleClickInterval->setValue(value);

  value = qtSettings->value(QLatin1String("wheelScrollLines"), 3).toInt();
  ui.wheelScrollLines->setValue(value);
  qtSettings->endGroup();
}


void MouseConfig::onMouseAccelChanged(int value) {
  QSlider* slider = static_cast<QSlider*>(sender());
  accel = value;
  XChangePointerControl(QX11Info::display(), True, False,
                        accel, 10, 0);
}

void MouseConfig::onMouseThresholdChanged(int value) {
  QSlider* slider = static_cast<QSlider*>(sender());
  /* threshold = 110 - sensitivity. The lower the threshold, the higher the sensitivity */
  threshold = 110 - value;
  XChangePointerControl(QX11Info::display(), False, True,
                        0, 10, threshold);
}

/* This function is taken from Gnome's control-center 2.6.0.3 (gnome-settings-mouse.c) and was modified*/
#define DEFAULT_PTR_MAP_SIZE 128
void MouseConfig::setLeftHandedMouse() {
  unsigned char* buttons;
  int n_buttons, i;
  int idx_1 = 0, idx_3 = 1;

  buttons = (unsigned char*)malloc(DEFAULT_PTR_MAP_SIZE);
  n_buttons = XGetPointerMapping(QX11Info::display(), buttons, DEFAULT_PTR_MAP_SIZE);

  if(n_buttons > DEFAULT_PTR_MAP_SIZE) {
    buttons = (unsigned char*)realloc(buttons, n_buttons);
    n_buttons = XGetPointerMapping(QX11Info::display(), buttons, n_buttons);
  }

  for(i = 0; i < n_buttons; i++) {
    if(buttons[i] == 1)
      idx_1 = i;
    else if(buttons[i] == ((n_buttons < 3) ? 2 : 3))
      idx_3 = i;
  }

  if((leftHanded && idx_1 < idx_3) ||
      (!leftHanded && idx_1 > idx_3)) {
    buttons[idx_1] = ((n_buttons < 3) ? 2 : 3);
    buttons[idx_3] = 1;
    XSetPointerMapping(QX11Info::display(), buttons, n_buttons);
  }
  free(buttons);
}

void MouseConfig::onMouseLeftHandedToggled(bool checked) {
  leftHanded = checked;
  setLeftHandedMouse();
}

void MouseConfig::onDoubleClickIntervalChanged(int value)
{
  qtSettings->beginGroup(QLatin1String("Qt"));
  qtSettings->setValue(QLatin1String("doubleClickInterval"), value);
  qtSettings->endGroup();
  qtSettings->sync();
#ifdef Q_WS_X11
  qt_x11_apply_settings_in_all_apps();
#endif
}

void MouseConfig::onWheelScrollLinesChanged(int value)
{
  qtSettings->beginGroup(QLatin1String("Qt"));
  qtSettings->setValue(QLatin1String("wheelScrollLines"), value);
  qtSettings->endGroup();
  qtSettings->sync();
#ifdef Q_WS_X11
  qt_x11_apply_settings_in_all_apps();
#endif
}

void MouseConfig::loadSettings() {
  settings->beginGroup("Mouse");
  oldAccel = accel = settings->value("acc_factor", 20).toInt();
  oldThreshold = threshold = settings->value("acc_threshold", 10).toInt();
  oldLeftHanded = leftHanded = settings->value("left_handed", false).toBool();
  settings->endGroup();
}

void MouseConfig::accept() {
  settings->beginGroup("Mouse");
  settings->setValue("acc_factor", accel);
  settings->setValue("acc_threshold", threshold);
  settings->setValue("left_handed", leftHanded);
  settings->endGroup();
}

void MouseConfig::reset() {
  /* restore to original settings */
  /* mouse */
  accel = oldAccel;
  threshold = oldThreshold;
  leftHanded = oldLeftHanded;
  XChangePointerControl(QX11Info::display(), True, True,
                        accel, 10, threshold);
  setLeftHandedMouse();

  initControls();
  accept();
}
