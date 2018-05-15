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
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QDateTime>

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
# GTK3 ignores bold or italic attributes.
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

ConfigOtherToolKits::ConfigOtherToolKits(LXQt::Settings *settings,  LXQt::Settings *configAppearanceSettings, QObject *parent) : QObject(parent)
{
    mSettings = settings;
    mConfigAppearanceSettings = configAppearanceSettings;
}

static QString get_environment_var(const char *envvar, const char *defaultValue)
{
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString mDirPath = QString::fromLocal8Bit(qgetenv(envvar));
    if(mDirPath.isEmpty())
        mDirPath = homeDir + defaultValue;
    else {
        for(QString path : mDirPath.split(":") ) {
            mDirPath = path;
            break;
        }
    }
    return mDirPath;
}

static QString _get_config_path(QString path)
{
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    path.replace("$XDG_CONFIG_HOME", get_environment_var("XDG_CONFIG_HOME", "/.config"));
    path.replace("$GTK2_RC_FILES",   get_environment_var("GTK2_RC_FILES", "/.gtkrc-2.0")); // If $GTK2_RC_FILES is undefined, "~/.gtkrc-2.0" will be used.
    path.replace("~", homeDir);
    return path;
}

QString ConfigOtherToolKits::getGTKConfigPath(QString version)
{
    if(version == "2.0")
        return _get_config_path("$GTK2_RC_FILES");
    return _get_config_path(QString("$XDG_CONFIG_HOME/gtk-%1/settings.ini").arg(version));
}

static bool grep(QFile &file, QByteArray text)
{
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    while (!file.atEnd()) {
        QByteArray line = file.readLine().trimmed();
        if(line.startsWith(text)) {
            return true;   
        }
    }
    file.close();
    return false;
}

bool ConfigOtherToolKits::backupGTKSettings(QString version)
{
        QString gtkrcPath = getGTKConfigPath(version);
        QFile file(gtkrcPath);
        if(file.exists() && !grep(file, "# Created by lxqt-config-appearance (DO NOT EDIT!)")) {
            file.copy(gtkrcPath + "-" + QString::number(QDateTime::currentSecsSinceEpoch()) + "~");
            return true;
        }
        return false;
}

void ConfigOtherToolKits::setConfig()
{
    if(!mConfigAppearanceSettings->contains("ControlGTKThemeEnabled"))
        mConfigAppearanceSettings->setValue("ControlGTKThemeEnabled", false);
    bool controlGTKThemeEnabled = mConfigAppearanceSettings->value("ControlGTKThemeEnabled").toBool();
    if(! controlGTKThemeEnabled)
        return;
    updateConfigFromSettings();
    mConfig.styleTheme = getGTKThemeFromRCFile("3.0");
    setGTKConfig("3.0");
    mConfig.styleTheme = getGTKThemeFromRCFile("2.0");
    setGTKConfig("2.0");
    setXSettingsConfig();
}

void ConfigOtherToolKits::setXSettingsConfig()
{
    updateConfigFromSettings();
    mConfig.styleTheme = getGTKThemeFromRCFile("3.0");
    if(mConfig.styleTheme.isEmpty())
        mConfig.styleTheme = getGTKThemeFromRCFile("2.0");
    writeConfig("~/.xsettingsd", XSETTINGS_CONFIG);
    // Reload settings. xsettingsd must be installed.
    QProcess::startDetached("xsettingsd");
}

void ConfigOtherToolKits::setGTKConfig(QString version, QString theme)
{
    updateConfigFromSettings();
    if(!theme.isEmpty())
        mConfig.styleTheme = theme;
    backupGTKSettings(version);
    QString gtkrcPath = getGTKConfigPath(version);
    if(version == "2.0")
        writeConfig(gtkrcPath, GTK2_CONFIG);
    else
        writeConfig(gtkrcPath, GTK3_CONFIG);
}

void ConfigOtherToolKits::writeConfig(QString path, const char *configString)
{
    path = _get_config_path(path);
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
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

QStringList ConfigOtherToolKits::getGTKThemes(QString version)
{
    QStringList themeList;
    QString configFile = version=="2.0" ? "gtk-2.0/gtkrc" : QString("gtk-%1/gtk.css").arg(version);

    QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for(QString dataPath : dataPaths) {
        QDir themesPath(dataPath + "/themes");
        QStringList themes = themesPath.entryList(QDir::Dirs);
        for(QString theme : themes) {
            QFileInfo themePath(QString("%1/themes/%2/%3").arg(dataPath, theme, configFile));
            if(themePath.exists())
                themeList.append(theme);
        }
    }
    return themeList;
}

QString ConfigOtherToolKits::getGTKThemeFromRCFile(QString version)
{
    if(version == "2.0") {
        QString gtkrcPath = _get_config_path("$GTK2_RC_FILES");
        QFile file(gtkrcPath);
        if(file.exists()) {
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return QString();
            while (!file.atEnd()) {
                QByteArray line = file.readLine().trimmed();
                if(line.startsWith("gtk-theme-name")) {
                    QList<QByteArray> parts = line.split('=');
                    if(parts.size()>=2) {
                        file.close();
                        return parts[1].replace('"', "").trimmed(); 
                    }   
                }
            }
            file.close();
        }
    } else {
        QString gtkrcPath = _get_config_path(QString("$XDG_CONFIG_HOME/gtk-%1/settings.ini").arg(version));
        QFile file(gtkrcPath);
        if(file.exists()) {
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return QString();
            bool settingsFound = false;
            while (!file.atEnd()) {
                QByteArray line = file.readLine().trimmed();
                if(line.startsWith("[Settings]"))
                    settingsFound = true;
                else if(line.startsWith("[") && line.endsWith("]"))
                    settingsFound = false;
                else if(settingsFound && line.startsWith("gtk-theme-name")) {
                    QList<QByteArray> parts = line.split('=');
                    if(parts.size()>=2) {
                        file.close();
                        return parts[1].trimmed();
                    }   
                }
            }
            file.close();
        }
    }
    return QString();
}

void ConfigOtherToolKits::updateConfigFromSettings()
{
    mSettings->beginGroup(QLatin1String("Qt"));
    QFont font;
    font.fromString(mSettings->value("font").toString());
    // Font name from: https://developer.gnome.org/pango/stable/pango-Fonts.html#pango-font-description-from-string
    // FAMILY-LIST [SIZE]", where FAMILY-LIST is a comma separated list of families optionally terminated by a comma, 
    // STYLE_OPTIONS is a whitespace separated list of words where each word describes one of style, variant, weight, stretch, or gravity, and 
    // SIZE is a decimal number (size in points) or optionally followed by the unit modifier "px" for absolute size. 
    mConfig.fontName = QString("%1%2%3 %4")
        .arg(font.family())                                 //%1
        .arg(font.style()==QFont::StyleNormal?"":" Italic") //%2
        .arg(font.weight()==QFont::Normal?"":" Bold")       //%3
        .arg(font.pointSize());                             //%4
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

