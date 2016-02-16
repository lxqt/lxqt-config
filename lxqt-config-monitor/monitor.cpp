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

#include "monitor.h"

bool MonitorSavedSettings::operator==(const MonitorSavedSettings &obj)
{
    if(name != obj.name)
        return false;
    if(date != obj.date)
        return false;
    // TODO: Check QList<MonitorSettings> monitors.
    return true;
}

void saveMonitorSettings(QSettings & settings, QList<MonitorSettings> monitors)
{
    settings.remove("settings");
    settings.beginWriteArray("settings");
    int i = 0;
    Q_FOREACH(MonitorSettings monitor, monitors)
    {
        settings.setArrayIndex(i++);
        saveMonitorSettings(settings, monitor);
    }
    settings.endArray();
}

void saveMonitorSettings(QSettings &settings, MonitorSettings &monitor)
{
    settings.setValue("name", monitor.name);
    settings.setValue("hash", monitor.hash);
    settings.setValue("connected", monitor.connected);
    if(monitor.connected)
    {
        settings.setValue("enabled", monitor.enabled);
        settings.setValue("primary", monitor.primary);
        settings.setValue("xPos", monitor.xPos);
        settings.setValue("yPos", monitor.yPos);
        settings.setValue("currentMode", monitor.currentMode);
        settings.setValue("currentModeWidth", monitor.currentModeWidth);
        settings.setValue("currentModeHeight", monitor.currentModeHeight);
        settings.setValue("currentModeRate", monitor.currentModeRate);
        settings.setValue("rotation", monitor.rotation);
    }
}

void loadMonitorSettings(QSettings & settings, QList<MonitorSettings> &monitors)
{
    int size = settings.beginReadArray("settings");
    for(int i=0; i<size; i++)
    {
        settings.setArrayIndex(i);
        MonitorSettings monitor;
        loadMonitorSettings(settings, monitor);
        monitors.append(monitor);
    }
    settings.endArray();
}

void loadMonitorSettings(QSettings &settings, MonitorSettings &monitor)
{
    monitor.name = settings.value("name").toString();
    monitor.hash = settings.value("hash").toString();
    monitor.connected = settings.value("connected").toBool();
    if(monitor.connected)
    {
        monitor.enabled = settings.value("enabled").toBool();
        monitor.primary = settings.value("primary").toBool();
        monitor.xPos = settings.value("xPos").toInt();
        monitor.yPos = settings.value("yPos").toInt();
        monitor.currentMode = settings.value("currentMode").toString();
        monitor.currentModeWidth = settings.value("currentModeWidth").toInt();
        monitor.currentModeHeight = settings.value("currentModeHeight").toInt();
        monitor.currentModeRate = settings.value("currentModeRate").toFloat();
        monitor.rotation = settings.value("rotation").toInt();
    }
}

void saveMonitorSettings(QSettings & settings, QList<MonitorSavedSettings> monitors)
{
    settings.remove("SavedSettings");
    settings.beginWriteArray("SavedSettings");
    int i = 0;
    Q_FOREACH(MonitorSavedSettings monitor, monitors)
    {
        settings.setArrayIndex(i++);
        saveMonitorSettings(settings, monitor);
    }
    settings.endArray();
}

void saveMonitorSettings(QSettings &settings, MonitorSavedSettings &monitor)
{
    settings.setValue("name", monitor.name);
    settings.setValue("date", monitor.date);
    saveMonitorSettings(settings, monitor.monitors);
}

void loadMonitorSettings(QSettings & settings, QList<MonitorSavedSettings> &monitors)
{
    int size = settings.beginReadArray("SavedSettings");
    for(int i=0; i<size; i++)
    {
        settings.setArrayIndex(i);
        MonitorSavedSettings monitor;
        loadMonitorSettings(settings, monitor);
        monitors.append(monitor);
    }
    settings.endArray();
}

void loadMonitorSettings(QSettings &settings, MonitorSavedSettings &monitor)
{
    monitor.name = settings.value("name").toString();
    monitor.date = settings.value("date").toString();
    loadMonitorSettings(settings, monitor.monitors);
}