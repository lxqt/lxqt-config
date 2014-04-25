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

#ifndef KEYBOARDLAYOUTCONFIG_H
#define KEYBOARDLAYOUTCONFIG_H

#include <QWidget>
#include <QList>

namespace LxQt {
  class Settings;
}

namespace Ui {
class KeyboardLayoutConfig;
}

struct LayoutVariant {
  QString name;
  QString description;
};

struct KeyboardLayout {
  QString lang;
  QString langDescription;
  QList<LayoutVariant> variants;
};

struct KeyboardModel {
  QString name;
  QString description;
};

class KeyboardLayoutConfig : public QWidget {
  Q_OBJECT
public:
  KeyboardLayoutConfig(LxQt::Settings* _settings, QWidget* parent = 0);
  virtual ~KeyboardLayoutConfig();
  void accept();

public Q_SLOTS:
  void reset();

private:
  void loadSettings();
  void loadLists();
  void initControls();

private:
  Ui::KeyboardLayoutConfig* ui;
  QString model_;
  QList<KeyboardLayout> layouts_;
  QList<KeyboardLayout> knownLayouts_;
  QList<KeyboardModel> knownModels_;
};

#endif // KEYBOARDLAYOUTCONFIG_H
