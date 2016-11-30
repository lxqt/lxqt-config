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

#include "touchpaddevice.h"

#include <QDebug>
#include <QX11Info>
#include <libudev.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <xorg/libinput-properties.h>

static QList<QVariant> xi2_get_device_property(int deviceid, const char* prop)
{
    QList<QVariant> ret;

    Display* dpy = QX11Info::display();

    Atom act_type;
    int act_format;
    unsigned long nitems, bytes_after;
    unsigned char *data, *ptr;
    Atom prop_atom = XInternAtom(dpy, prop, False);
    XIGetProperty(dpy, deviceid, prop_atom, 0, 1000, False, AnyPropertyType,
                  &act_type, &act_format, &nitems, &bytes_after, &data);
    ptr = data;
    for (unsigned long i = 0; i < nitems; i++)
    {
        switch (act_type)
        {
        case XA_INTEGER:
            switch (act_format)
            {
            case 8:
                ret << *reinterpret_cast<quint8*>(ptr);
                break;
            case 16:
                ret << *reinterpret_cast<quint16*>(ptr);
                break;
            case 32:
                ret << *reinterpret_cast<quint32*>(ptr);
                break;
            default:
                Q_ASSERT(0);
            }
            ptr += act_format / 8;
            break;
        case XA_STRING:
        {
            Q_ASSERT(act_format == 8);
            QString s(reinterpret_cast<char*>(ptr));
            ptr += s.size() + 1; // including '\0'
            ret << s;
            break;
        }
        default:
            qWarning() << "Unrecognized type " << act_type << " for property " << prop;
        }
    }

    XFree(data);

    return ret;
}

static bool xi2_set_device_property(int deviceid, const char* prop, QList<QVariant> values)
{
    Display* dpy = QX11Info::display();

    Atom prop_atom = XInternAtom(dpy, prop, False);

    Atom act_type;
    int act_format;
    unsigned long nitems, bytes_after;
    unsigned char *data;
    // get the property first to determine its type and format
    XIGetProperty(dpy, deviceid, prop_atom, 0, 1000, False, AnyPropertyType,
                  &act_type, &act_format, &nitems, &bytes_after, &data);
    if (!nitems)
    {
        return false;
    }

    XFree(data);

    switch (values[0].type())
    {
    case QVariant::Int:
        Q_ASSERT(act_type == XA_INTEGER);

        data = new unsigned char[values.size() * act_format / 8];
        for (int i = 0; i < values.size(); i++)
        {
            switch (act_format)
            {
            case 8:
                *reinterpret_cast<int8_t*>(data + i) = values[i].toInt();
                break;
            case 16:
                *reinterpret_cast<int16_t*>(data + i) = values[i].toInt();
                break;
            case 32:
                *reinterpret_cast<int32_t*>(data + i) = values[i].toInt();
                break;
            default:
                Q_ASSERT(0);
            }
        }
        break;
    default:
        qWarning("Unsupported data type");
    }
    if (!data)
    {
        return false;
    }
    XIChangeProperty(dpy, deviceid, prop_atom, act_type, act_format,
                     XIPropModeReplace, data, values.size());
    // XXX How to catch errors?
    return true;
}

QList<QVariant> TouchpadDevice::get_xi2_property(const char* prop) const
{
    Q_ASSERT(deviceid);

    return xi2_get_device_property(deviceid, prop);
}

bool TouchpadDevice::set_xi2_property(const char* prop, QList<QVariant> values)
{
    Q_ASSERT(deviceid);

    return xi2_set_device_property(deviceid, prop, values);
}

QList<TouchpadDevice> TouchpadDevice::enumerate_from_udev()
{
    QList<TouchpadDevice> ret;
    udev *ud = udev_new();

    if (!ud)
    {
        qWarning() << "Fails to initialize udev";
        return ret;
    }
    
    udev_enumerate *enumerate = udev_enumerate_new(ud);
    udev_enumerate_add_match_property(enumerate, "ID_INPUT_TOUCHPAD", "1");
    udev_enumerate_scan_devices(enumerate);
    udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    for (udev_list_entry *entry = devices; entry; entry = udev_list_entry_get_next(entry)) {
        const char *path = udev_list_entry_get_name(entry);
        udev_device *dev = udev_device_new_from_syspath(ud, path);

        const char* devnode = udev_device_get_devnode(dev);
        if (devnode)
        {
            TouchpadDevice dev;
            dev.devnode = devnode;
            if(dev.find_xi2_device())
            {
                qDebug() << "Detected" << dev.m_name << "on" << dev.devnode;
                dev.m_oldTappingEnabled = dev.tappingEnabled();
                dev.m_oldNaturalScrollingEnabled = dev.naturalScrollingEnabled();
                dev.m_oldScrollingMethodEnabled = dev.scrollingMethodEnabled();
                ret << dev;
            }
        }

        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
    udev_unref(ud);

    return ret;
}


bool TouchpadDevice::find_xi2_device()
{
    Display* dpy = QX11Info::display();

    int ndevices;
    bool found = false;
    XIDeviceInfo *info = XIQueryDevice(dpy, XIAllDevices, &ndevices);

    for(int i = 0; i < ndevices ; i++)
    {
        QList<QVariant> devnode_prop = xi2_get_device_property(info[i].deviceid, "Device Node");
        if (devnode_prop.size() && devnode_prop[0].toString() == devnode)
        {
            m_name = info[i].name;
            deviceid = info[i].deviceid;
            found = true;
            break;
        }
    }

    XIFreeDeviceInfo(info);

    return found;
}

int TouchpadDevice::featureEnabled(const char* prop) const
{
    QList<QVariant> propVal = get_xi2_property(prop);
    if (propVal.size())
    {
        return propVal[0].toInt();
    }
    else
    {
        return -1;
    }
}

int TouchpadDevice::tappingEnabled() const
{
    return featureEnabled(LIBINPUT_PROP_TAP);
}

int TouchpadDevice::naturalScrollingEnabled() const
{
    return featureEnabled(LIBINPUT_PROP_NATURAL_SCROLL);
}

bool TouchpadDevice::setTappingEnabled(bool enabled)
{
    return set_xi2_property(LIBINPUT_PROP_TAP, QList<QVariant>({enabled ? 1 : 0}));
}

bool TouchpadDevice::setNaturalScrollingEnabled(bool enabled)
{
    return set_xi2_property(LIBINPUT_PROP_NATURAL_SCROLL, QList<QVariant>({enabled ? 1 : 0}));
}

int TouchpadDevice::scrollMethodsAvailable() const
{
    QList<QVariant> values = get_xi2_property(LIBINPUT_PROP_SCROLL_METHODS_AVAILABLE);

    Q_ASSERT(values.size() == 3);

    return (values[0].toInt() ? TWO_FINGER : 0) |
           (values[1].toInt() ? EDGE : 0) |
           (values[2].toInt() ? BUTTON : 0);
}

ScrollingMethod TouchpadDevice::scrollingMethodEnabled() const
{
    QList<QVariant> values = get_xi2_property(LIBINPUT_PROP_SCROLL_METHOD_ENABLED);

    Q_ASSERT(values.size() == 3);
    
    // those methods are mutually exclusive
    if (values[0].toInt())
        return TWO_FINGER;
    else if (values[1].toInt())
        return EDGE;
    else if (values[2].toInt())
        return BUTTON;
    else
        return NONE;
}

bool TouchpadDevice::setScrollingMethodEnabled(ScrollingMethod method)
{
    QList<QVariant> values;

    values << ((method == TWO_FINGER) ? 1 : 0);
    values << ((method == EDGE) ? 1 : 0);
    values << ((method == BUTTON) ? 1 : 0);

    return set_xi2_property(LIBINPUT_PROP_SCROLL_METHOD_ENABLED, values);
}
