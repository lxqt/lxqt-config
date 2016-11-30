/*
    Copyright (C) 2016 Yen Chi Hsuan <yan12125@gmail.com>

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

#ifndef TOUCHPADDEVICE_H
#define TOUCHPADDEVICE_H

#include <QList>
#include <QVariant>

enum ScrollingMethod
{
    NONE = 0,
    TWO_FINGER = 1,
    EDGE = 2,
    BUTTON = 4
};

class TouchpadDevice
{
public:
    TouchpadDevice(): deviceid(0) {}

    static QList<TouchpadDevice> enumerate_from_udev();

    const QString& name() const { return m_name; }

    int tappingEnabled() const;
    int naturalScrollingEnabled() const;
    bool setTappingEnabled(bool enabled);
    bool setNaturalScrollingEnabled(bool enabled);
    bool oldTappingEnabled() const { return m_oldTappingEnabled; }
    bool oldNaturalScrollingEnabled() const { return m_oldNaturalScrollingEnabled; }
    ScrollingMethod oldScrollingMethodEnabled() const { return m_oldScrollingMethodEnabled; }

    int scrollMethodsAvailable() const;
    ScrollingMethod scrollingMethodEnabled() const;
    bool setScrollingMethodEnabled(ScrollingMethod method);
private:
    QString m_name;
    QString devnode;
    int deviceid;

    bool m_oldTappingEnabled;
    bool m_oldNaturalScrollingEnabled;
    ScrollingMethod m_oldScrollingMethodEnabled;

    QList<QVariant> get_xi2_property(const char* prop) const;
    bool set_xi2_property(const char* prop, QList<QVariant> values);
    bool find_xi2_device();
    int featureEnabled(const char* prop) const;
};

#endif
