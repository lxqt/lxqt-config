/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "keyboardlayoutconfig.h"
#include "ui_keyboardlayoutconfig.h"
#include <QProcess>
#include <QFile>
#include <QDebug>

KeyboardLayoutConfig::KeyboardLayoutConfig(LxQt::Settings* _settings, QWidget* parent) {
  ui = new Ui::KeyboardLayoutConfig;
  ui->setupUi(this);

  loadLists();
  loadSettings();
  initControls();
}

KeyboardLayoutConfig::~KeyboardLayoutConfig() {
  delete ui;
}

void KeyboardLayoutConfig::loadSettings() {
  // load current settings from the output of setxkbmap command
  QProcess setxkbmap;
  setxkbmap.start(QLatin1String("setxkbmap -query -verbose 5"));
  setxkbmap.waitForFinished();
  if(setxkbmap.exitStatus() == QProcess::NormalExit) {
    while(!setxkbmap.atEnd()) {
      QByteArray line = setxkbmap.readLine();
      if(line.startsWith("model:")) {
        model_ = QString::fromLatin1(line.mid(6).trimmed());
      }
      else if(line.startsWith("layout:")) {
        QList<QByteArray> items = line.mid(7).trimmed().split(',');
        Q_FOREACH(QByteArray item, items) {
          QString lang;
	  QString variant;
          int p = item.indexOf('(');
          if(p >= 0) { // has variant
            int p2 = item.lastIndexOf(')');
            if(p2 >= 0)
              variant = QString::fromLatin1(item.mid(p + 1, (p2 - p - 1)));
            lang = QString::fromLatin1(item.left(p));
          }
          else
            lang = QString::fromLatin1(item);
          // layouts_.append(layoutItem);
        }
      }
    }
    setxkbmap.close();
  }
}

enum ListSection{
  NoSection,
  ModelSection,
  LayoutSection,
  VariantSection,
  OptionSection
};

void KeyboardLayoutConfig::loadLists() {
  // load known lists from xkb data files
  // FIXME: maybe we should use different files for different OSes?
  QFile file(QLatin1String("/usr/share/X11/xkb/rules/base.lst"));
  if(file.open(QIODevice::ReadOnly)) {
    ListSection section = NoSection;
    while(!file.atEnd()) {
      QByteArray line = file.readLine().trimmed();
      if(section == NoSection) {
        if(line.startsWith("! model"))
          section = ModelSection;
        else if(line.startsWith("! layout"))
          section = LayoutSection;
        else if(line.startsWith("! variant"))
          section = VariantSection;
        else if(line.startsWith("! option"))
          section = OptionSection;
      }
      else {
        if(line.isEmpty()) {
          section = NoSection;
	  continue;
	}
	switch(section) {
	  case ModelSection: {
	    KeyboardModel model;
	    model.name = QString::fromLatin1(line);
	    knownModels_.append(model);
	    break;
	  }
	  case LayoutSection:
	    break;
	  case VariantSection:
	    break;
	  case OptionSection:
	    break;
	}
      }
    }
    file.close();
  }
}

void KeyboardLayoutConfig::initControls() {

}

void KeyboardLayoutConfig::reset() {

}

void KeyboardLayoutConfig::accept() {

}
