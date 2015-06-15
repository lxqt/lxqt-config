/*
    Copyright (C) 2014  P.L. Lucas <selairi@gmail.com>
    Copyright (C) 2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

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

#ifndef _MONITORWIDGET_H_
#define _MONITORWIDGET_H_

#include "ui_monitorwidget.h"

#include <QGroupBox>
#include <QStringList>
#include <QHash>
#include <QList>
#include <KScreen/Config>
#include <KScreen/Output>

#define PrimaryDisplay 0
#define ExtendDisplay 1
#define CloneDisplay 2

#define RightOf 0
#define LeftOf 1
#define Above 2
#define Below 3
#define Manually 4

class MonitorWidget : public QGroupBox
{
  Q_OBJECT

  friend class MonitorPicture;
  friend class MonitorPictureDialog;

public:
    MonitorWidget(KScreen::OutputPtr output, KScreen::ConfigPtr config, QWidget* parent = 0);
    ~MonitorWidget();

    void updateRefreshRates();

    KScreen::OutputPtr output;
    KScreen::ConfigPtr config;

public Q_SLOTS:
    void setOnlyMonitor(bool isOnlyMonitor);

private Q_SLOTS:
    void onEnabledChanged(bool);
    void onBehaviorChanged(int);
    void onPositioningChanged(int);
    void onPositionChanged(int);
    void onResolutionChanged(int);
    void onRateChanged(int);
    void onOrientationChanged(int);
    void onCloneChanged(int);

private:
    Ui::MonitorWidget ui;
};

#endif // _MONITORWIDGET_H_
