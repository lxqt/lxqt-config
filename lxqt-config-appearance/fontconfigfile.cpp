/*
 * Copyright (C) 2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 * LXQt project: https://lxqt-project.org/
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

#include "fontconfigfile.h"
#include <QTextStream>
#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QStringBuilder>
#include <QDomDocument>
#include <QTimer>
#include <QDebug>
#include <QStandardPaths>

#include <LXQt/Settings>

using namespace Qt::Literals::StringLiterals;

FontConfigFile::FontConfigFile(LXQt::Settings *settings):
    mSettings(settings)
{
    mDirPath = QString::fromLocal8Bit(qgetenv("XDG_CONFIG_HOME"));
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if(mDirPath.isEmpty())
        mDirPath = homeDir + QStringLiteral("/.config");
    mDirPath += QLatin1String("/fontconfig");
    mFilePath = mDirPath + QStringLiteral("/fonts.conf");
}

void FontConfigFile::save()
{
    QFile file(mFilePath);
    QDir().mkdir(mDirPath);
    // References: https://wiki.archlinux.org/index.php/Font_Configuration
    if(!file.open(QIODevice::WriteOnly))
        return;

    bool antialias = mSettings->value("Fonts/Antialias"_L1, true).toBool();
    QByteArray subpixel = mSettings->value("Fonts/Subpixel"_L1, "rgb"_ba).toByteArray();
    bool hinting = mSettings->value("Fonts/Hinting"_L1, true).toBool();
    QByteArray hintStyle = mSettings->value("Fonts/Hintstyle"_L1, "hintslight"_ba).toByteArray();
    bool autohint = mSettings->value("Fonts/Autohint"_L1, false).toBool();
    int dpi = mSettings->value("Fonts/DPI"_L1, 96).toInt();

    QTextStream s(&file);
    s <<
    "<?xml version=\"1.0\"?>\n"
    "<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">\n"
    "<!-- created by lxqt-config-appearance (DO NOT EDIT!) -->\n"
    "<fontconfig>\n"
    "  <include ignore_missing=\"yes\">conf.d</include>\n"
    "  <match target=\"font\">\n"
    "    <edit name=\"antialias\" mode=\"assign\">\n"
    "      <bool>" << (antialias ? "true" : "false") << "</bool>\n"
    "    </edit>\n"
    "  </match>\n"
    "  <match target=\"font\">\n"
    "    <edit name=\"rgba\" mode=\"assign\">\n"
    "      <const>" << subpixel << "</const>\n"
    "    </edit>\n"
    "  </match>\n"
    "  <match target=\"font\">\n"
    "    <edit name=\"lcdfilter\" mode=\"assign\">\n"
    "      <const>lcddefault</const>\n"
    "    </edit>\n"
    "  </match>\n"
    "  <match target=\"font\">\n"
    "    <edit name=\"hinting\" mode=\"assign\">\n"
    "      <bool>" << (hinting ? "true" : "false") << "</bool>\n"
    "    </edit>\n"
    "  </match>\n"
    "  <match target=\"font\">\n"
    "    <edit name=\"hintstyle\" mode=\"assign\">\n"
    "      <const>" << hintStyle << "</const>\n"
    "    </edit>\n"
    "  </match>\n"
    "  <match target=\"font\">\n"
    "    <edit name=\"autohint\" mode=\"assign\">\n"
    "      <bool>" << (autohint ? "true" : "false") << "</bool>\n"
    "    </edit>\n"
    "  </match>\n"
    "  <match target=\"pattern\">\n"
    "    <edit name=\"dpi\" mode=\"assign\">\n"
    "      <double>" << dpi << "</double>\n"
    "    </edit>\n"
    "  </match>\n"
    "</fontconfig>";
}
