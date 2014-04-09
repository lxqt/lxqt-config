/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2012 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef FONTSCONFIG_H
#define FONTSCONFIG_H

#include <QWidget>
#include <QFont>
#include <lxqt/LxQtSettings>

class QTreeWidgetItem;
class QSettings;

namespace Ui {
    class FontsConfig;
}

class FontsConfig : public QWidget
{
    Q_OBJECT

public:
    explicit FontsConfig(LxQt::Settings *settings, QSettings *qtSettings, QWidget *parent = 0);
    ~FontsConfig();

public Q_SLOTS:
    void initControls();

private Q_SLOTS:
  void fontChanged();
  //void fontChanged(const QFont & font);
  //void fontStyleChanged(int index);
  //void fontSizeChanged(int value);

  void antialiasToggled(bool toggled);
  void hintingToggled(bool toggled);
  void subpixelChanged(int index);
  void hintStyleChanged(int index);
  void dpiChanged(int value);

private:
    Ui::FontsConfig *ui;
    QSettings *mQtSettings;
    LxQt::Settings *mSettings;
    LxQt::Settings mSessionSettings;
};

#endif // FONTSCONFIG_H
