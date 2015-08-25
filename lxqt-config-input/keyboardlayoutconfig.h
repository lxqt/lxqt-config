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
#include "keyboardlayoutinfo.h"
#include <QMap>
#include "ui_keyboardlayoutconfig.h"

namespace LXQt {
  class Settings;
}

class KeyboardLayoutConfig : public QWidget {
  Q_OBJECT
public:
  KeyboardLayoutConfig(LXQt::Settings* _settings, QWidget* parent = 0);
  virtual ~KeyboardLayoutConfig();

public Q_SLOTS:
  void accept();
  void reset();
  void onAddLayout();
  void onRemoveLayout();
  void onMoveUp();
  void onMoveDown();

private:
  void loadSettings();
  void loadLists();
  void initControls();
  void addLayout(QString name, QString variant);

private:
  Ui::KeyboardLayoutConfig ui;
  QString keyboardModel_;
  QString switchKey_;
  QStringList currentOptions_;
  QList<QPair<QString, QString> > currentLayouts_;
  QMap<QString, KeyboardLayoutInfo> knownLayouts_;
  LXQt::Settings* settings;
};

#endif // KEYBOARDLAYOUTCONFIG_H
