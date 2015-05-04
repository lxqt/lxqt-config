/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://sourceforge.net/projects/lxde-qt/
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *   Luis Pereira <luis.artur.pereira@gmail.com>
 *
 * The directoryMatchesSize() and thedirectorySizeDistance() functions were
 * taken from Qt5 qtbase/src/gui/image/qiconloader.cpp
 * Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include "iconthemeinfo.h"
#include <QDebug>

#include <private/qtxdg/qiconloader_p.h>

#include <QStringBuilder>

#define PREVIEW_ICON_SIZE 22

using namespace QtXdg;

/*
 * This algorithm is defined by the freedesktop spec:
 * http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
 */
static bool directoryMatchesSize(const QIconDirInfo &dir, int iconsize)
{
    if (dir.type == QIconDirInfo::Fixed) {
        return dir.size == iconsize;

    } else if (dir.type == QIconDirInfo::Scalable) {
        return dir.size <= dir.maxSize &&
                iconsize >= dir.minSize;

    } else if (dir.type == QIconDirInfo::Threshold) {
        return iconsize >= dir.size - dir.threshold &&
                iconsize <= dir.size + dir.threshold;
    }

    Q_ASSERT(1); // Not a valid value
    return false;
}


/*
 * This algorithm is defined by the freedesktop spec:
 * http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
 */
static int directorySizeDistance(const QIconDirInfo &dir, int iconsize)
{
    if (dir.type == QIconDirInfo::Fixed) {
        return qAbs(dir.size - iconsize);

    } else if (dir.type == QIconDirInfo::Scalable) {
        if (iconsize < dir.minSize)
            return dir.minSize - iconsize;
        else if (iconsize > dir.maxSize)
            return iconsize - dir.maxSize;
        else
            return 0;

    } else if (dir.type == QIconDirInfo::Threshold) {
        if (iconsize < dir.size - dir.threshold)
            return dir.minSize - iconsize;
        else if (iconsize > dir.size + dir.threshold)
            return iconsize - dir.maxSize;
        else return 0;
    }

    Q_ASSERT(1); // Not a valid value
    return INT_MAX;
}



IconThemeInfo::IconThemeInfo(const QDir &dir):
    mValid(false),
    mHidden(false)
{
    mName = dir.dirName();
    if (dir.exists(QStringLiteral("index.theme")))
        load(dir.absoluteFilePath(QStringLiteral("index.theme")));
}


void IconThemeInfo::load(const QString &fileName)
{
    mFileName = fileName;
    mValid = false;
    QSettings file(mFileName, QSettings::IniFormat);
    if (file.status() != QSettings::NoError)
        return;

    if (file.value(QStringLiteral("Icon Theme/Directories")).toStringList().join(QLatin1Char(' ')).isEmpty())
        return;

    mHidden = file.value(QStringLiteral("Icon Theme/Hidden"), false).toBool();
    mText = file.value(QStringLiteral("Icon Theme/Name")).toString();
    mComment = file.value(QStringLiteral("Icon Theme/Comment")).toString();

    mValid = true;
}


QVector<QIcon> IconThemeInfo::icons(const QStringList &iconNames) const
{
    QVector<QIcon> icons;

    const QString currentThemeName = QIconLoader::instance()->themeName();
    QIconLoader::instance()->setThemeName(mName);
    foreach (const QString &i, iconNames) {
        QThemeIconInfo info = QIconLoader::instance()->loadIcon(i);
        if (!info.entries.isEmpty()) {
            const int numEntries = info.entries.size();

            // Search for exact matches first
            bool found = false;
            for (int i = 0; i < numEntries; ++i) {
                QIconLoaderEngineEntry *entry = info.entries.at(i);
                    icons.append(QIcon(entry->filename));
                    found = true;
                    break;
                }
            if (!found) {  // No exact match. Search for an approximation
                // Find the minimum distance icon
                int minimalSize = INT_MAX;
                QIconLoaderEngineEntry *closestMatch = 0;
                for (int i = 0; i < numEntries; ++i) {
                    QIconLoaderEngineEntry *entry = info.entries.at(i);
                    int distance = directorySizeDistance(entry->dir, PREVIEW_ICON_SIZE);
                    if (distance < minimalSize) {
                        minimalSize  = distance;
                        closestMatch = entry;
                    }
                }
                icons.append(QIcon(closestMatch->filename));
            }
        } else {
            icons.append(QIcon());
        }
    }
    QIconLoader::instance()->setThemeName(currentThemeName);
    return icons;
}
