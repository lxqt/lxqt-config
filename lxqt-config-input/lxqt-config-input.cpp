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

#include <lxqt/lxqtapplication.h>
#include "lxqttranslate.h"
#include <lxqt/LxQtConfigDialog>
#include <lxqt/LxQtSettings>
#include "mouseconfig.h"
#include "keyboardconfig.h"
#include "../lxqt-config-cursor/selectwnd.h"

int main(int argc, char** argv) {
  LxQt::Application app(argc, argv);
  TRANSLATE_APP;

  QByteArray configName = qgetenv("LXQT_SESSION_CONFIG");
  if(configName.isEmpty())
    configName = "session";
  LxQt::Settings settings(configName);
  LxQt::ConfigDialog dlg(QObject::tr("Input Device Configurations"), &settings);

  MouseConfig* mouseConfig = new MouseConfig(&settings, &dlg);
  dlg.addPage(mouseConfig, QObject::tr("Mouse"), "input-mouse");
  QObject::connect(&dlg, SIGNAL(reset()), mouseConfig, SLOT(reset()));

  SelectWnd* cursorConfig = new SelectWnd(&settings, &dlg);
  cursorConfig->setCurrent();
  dlg.addPage(cursorConfig, QObject::tr("Mouse Cursor"), "preferences-desktop-theme");
  
  KeyboardConfig* keyboardConfig = new KeyboardConfig(&settings, &dlg);
  dlg.addPage(keyboardConfig, QObject::tr("Keyboard"), "input-keyboard");
  QObject::connect(&dlg, SIGNAL(reset()), keyboardConfig, SLOT(reset()));
 
  dlg.exec();
  return 0;
}
