/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org/
 *
 * Copyright: 2018 LXQt team
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

#include "configothertoolkits.h"
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QProcess>
#include <QMetaEnum>
#include <QToolBar>

static const char *GTK2_CONFIG = R"GTK2_CONFIG(
# Created by lxqt-config-appearance (DO NOT EDIT!)
gtk-theme-name = "%1"
gtk-icon-theme-name = "%2"
gtk-font-name = "%3"
gtk-button-images = %4
gtk-menu-images = %4
gtk-toolbar-style = %5
)GTK2_CONFIG";

static const char *GTK3_CONFIG = R"GTK3_CONFIG(
# Created by lxqt-config-appearance (DO NOT EDIT!)
[Settings]
gtk-theme-name = %1
gtk-icon-theme-name = %2
gtk-font-name = %3
gtk-menu-images = %4
gtk-button-images = %4
gtk-toolbar-style = %5
)GTK3_CONFIG";

static const char *XSETTINGS_CONFIG = R"XSETTINGS_CONFIG(
# Created by lxqt-config-appearance (DO NOT EDIT!)
Net/IconThemeName "%2"
Net/ThemeName "%1"
Gtk/FontName "%3"
Gtk/MenuImages %4
Gtk/ButtonImages %4
Gtk/ToolbarStyle "%5"
)XSETTINGS_CONFIG";

ConfigOtherToolKits::ConfigOtherToolKits(LXQt::Settings *settings, QObject *parent) : QObject(parent)
{
    mSettings = settings;
}

void ConfigOtherToolKits::setConfig()
{
    updateConfigFromSettings();
    writeConfig("~/.gtkrc-2.0", GTK2_CONFIG);
    writeConfig("$XDG_CONFIG_HOME/gtk-3.0/settings.ini", GTK3_CONFIG);
    writeConfig("~/.xsettingsd", XSETTINGS_CONFIG);
    // Reload settings. xsettingsd must be installed.
    QProcess::startDetached("xsettingsd");
}

void ConfigOtherToolKits::writeConfig(QString path, const char *configString)
{
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    
    QString mDirPath = QString::fromLocal8Bit(qgetenv("XDG_CONFIG_HOME"));
    if(mDirPath.isEmpty())
        mDirPath = homeDir + "/.config";
    path.replace("$XDG_CONFIG_HOME", mDirPath);
    path.replace("~", homeDir);
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        #include <stdio.h>
        printf("No se ha podido abrir: %s\n", path.toLocal8Bit().data());
        return;
    }
    QTextStream out(&file);
    out << QString(configString).arg(mConfig.styleTheme, mConfig.iconTheme,
        mConfig.fontName, mConfig.buttonStyle==0 ? "0":"1",
        mConfig.toolButtonStyle 
        );
    out.flush();
    file.close();
}

void ConfigOtherToolKits::updateConfigFromSettings()
{
    mSettings->beginGroup(QLatin1String("Qt"));
    mConfig.styleTheme = mSettings->value("style").toString();
    mConfig.fontName = mSettings->value("font").toString();
    mSettings->endGroup();
    mConfig.iconTheme = mSettings->value("icon_theme").toString();
    {
        // Tool button style
        QByteArray tb_style = mSettings->value("tool_button_style").toByteArray();
        // convert toolbar style name to value
        QMetaEnum me = QToolBar::staticMetaObject.property(QToolBar::staticMetaObject.indexOfProperty("toolButtonStyle")).enumerator();
        int val = me.keyToValue(tb_style.constData());
        mConfig.buttonStyle = 1;
        switch(val) {
            case Qt::ToolButtonIconOnly:
                mConfig.toolButtonStyle = "GTK_TOOLBAR_ICONS";
                break;
            case Qt::ToolButtonTextOnly:
                mConfig.toolButtonStyle = "GTK_TOOLBAR_TEXT";
                mConfig.buttonStyle = 0;
                break;
            case Qt::ToolButtonTextUnderIcon:
                mConfig.toolButtonStyle = "GTK_TOOLBAR_BOTH";
                break;
            default:
                mConfig.toolButtonStyle = "GTK_TOOLBAR_BOTH_HORIZ";
        }
    }
    
}
