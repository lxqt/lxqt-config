/*
 * Copyright (C) 2014  LXQt Team
 * This file is part of the LXQt project. <https://lxqt-project.org>
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

#include <QtCore/QtGlobal>
#ifdef Q_OS_LINUX
#define XKBD_BASELIST_PATH "/usr/share/X11/xkb/rules/base.lst"
#elif defined(Q_OS_FREEBSD)
#define XKBD_BASELIST_PATH "/usr/local/share/X11/xkb/rules/base.lst"
#elif defined(Q_OS_OPENBSD)
#define XKBD_BASELIST_PATH "/usr/X11R6/share/X11/xkb/rules/base.lst"
#else
#define XKBD_BASELIST_PATH "/usr/local/share/X11/xkb/rules/base.lst"
#endif

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
  KeyboardLayoutConfig(LXQt::Settings* _settings, QWidget* parent = nullptr);
  virtual ~KeyboardLayoutConfig();

  void applyConfig();

public Q_SLOTS:
  void reset();
  void onAddLayout();
  void onRemoveLayout();
  void onMoveUp();
  void onMoveDown();

Q_SIGNALS:
    void settingsChanged();

private:
  void loadSettings();
  void loadLists();
  void initControls();
  void addLayout(QString name, QString variant);
  void warnSetXkbMapNotFound();

private:
  Ui::KeyboardLayoutConfig ui;
  QString keyboardModel_;
  QString switchKey_;
  QStringList currentOptions_;
  QList<QPair<QString, QString> > currentLayouts_;
  QMap<QString, KeyboardLayoutInfo> knownLayouts_;
  LXQt::Settings* settings;
  bool applyConfig_;

  static constexpr int TimeoutSetXkbMap_msec = 100;
};

#endif // KEYBOARDLAYOUTCONFIG_H
