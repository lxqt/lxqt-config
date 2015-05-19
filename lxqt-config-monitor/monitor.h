/*
    Copyright (C) 2014  P.L. Lucas <selairi@gmail.com>

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

#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <QObject>
#include <QStringList>
#include <QHash>
#include <QList>
#include <QRect>
#include <KScreen/Config>

//Settings to pass to backend
class MonitorSettings : public QObject
{
    Q_OBJECT

public:
    MonitorSettings(QObject* parent = 0);
    QString name;
    QString currentMode;
    QString currentRate;
    QString gamma;
    int xPos;
    int yPos;
    bool enabledOk;
    enum Position {None = 0, Manual};
    Position position;
    bool primaryOk;

    QSize currentSize();
    QRect geometry();
};

// Monitor information from backend
class MonitorInfo : public MonitorSettings
{
    Q_OBJECT

public:
    KScreen::ConfigPtr mConfig;

    MonitorInfo(QObject* parent = 0);
    QStringList modes; // Modes of this monitor in order
    QHash<QString, QStringList> modeLines; // Rates suported by each mode
    QString preferredMode;
    QString preferredRate;
    QString edid; // EDID data, not used yet, can be used to detect vendor name of the monitor
    QString vendor;

    static bool LVDS_Ok; // Is true if LVDS (Laptop monitor) is connected.
    QString humanReadableName();
};

// Gets size from string rate. String rate format is "widthxheight". Example: 800x600
QSize sizeFromString(QString str);

#endif // _MONITOR_H_
