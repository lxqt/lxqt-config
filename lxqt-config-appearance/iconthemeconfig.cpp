/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Petr Vanek <petr@scribus.info>
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

#include "iconthemeconfig.h"

#include <XdgDesktopFile>
#include <XdgIcon>
#include <LXQt/Settings>
#include <QStringList>
#include <QStringBuilder>
#include <QIcon>
#include <QDebug>

#include <private/qtxdg/qiconloader_p.h>

using namespace QtXdg;

IconThemeConfig::IconThemeConfig(LXQt::Settings* settings, QWidget* parent):
    QWidget(parent),
    m_settings(settings)
{
    setupUi(this);

    initIconsThemes();
    initControls();
    connect(iconThemeList, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(iconThemeSelected(QTreeWidgetItem*,int)));

    connect(LXQt::Settings::globalSettings(), SIGNAL(settingsChanged()),
            this, SLOT(update()));
}


void IconThemeConfig::initIconsThemes()
{
    QStringList processed;
    QStringList baseDirs = QIcon::themeSearchPaths();
    static const QStringList iconNames = QStringList()
                    << QStringLiteral("document-open")
                    << QStringLiteral("document-new")
                    << QStringLiteral("edit-undo")
                    << QStringLiteral("media-playback-start");

    const int iconNamesN = iconNames.size();
    iconThemeList->setColumnCount(iconNamesN + 2);

    QList<QTreeWidgetItem *> items;
    foreach (QString baseDirName, baseDirs)
    {
        QDir baseDir(baseDirName);
        if (!baseDir.exists())
            continue;

        QFileInfoList dirs = baseDir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);
        foreach (QFileInfo dir, dirs)
        {
            if (!processed.contains(dir.canonicalFilePath()))
            {
                processed << dir.canonicalFilePath();

                IconThemeInfo theme(QDir(dir.canonicalFilePath()));
                if (theme.isValid() && (!theme.isHidden()))
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0);
                    item->setSizeHint(0, QSize(42,42)); // make icons non-cropped
                    item->setData(0, Qt::UserRole, theme.name());

                    const QVector<QIcon> icons = theme.icons(iconNames);

                    const int K = icons.size();
                    for (int i = 0; i < K; ++i)
                    {
                        item->setIcon(i, icons.at(i));
                    }

                    QString themeDescription;
                    if (theme.comment().isEmpty())
                    {
                        themeDescription = theme.text();
                    }
                    else
                    {
                        themeDescription = theme.text() % QStringLiteral(" (") % theme.comment() % QStringLiteral(")");
                    }

                    item->setText(iconNamesN + 1, themeDescription);

                    items.append(item);
                }
            }
        }
    }
    QIconLoader::instance()->updateSystemTheme();

    iconThemeList->insertTopLevelItems(0, items);
    for (int i=0; i<iconThemeList->header()->count()-1; ++i)
    {
        iconThemeList->resizeColumnToContents(i);
    }
}


void IconThemeConfig::initControls()
{
    QString currentTheme = LXQt::Settings::globalSettings()->value("icon_theme").toString();
    XdgIcon::setThemeName(currentTheme);
    QTreeWidgetItemIterator it(iconThemeList);
    while (*it) {
        if ((*it)->data(0, Qt::UserRole).toString() == currentTheme)
        {
            iconThemeList->setCurrentItem((*it));
            break;
        }
        ++it;
    }

    update();
}


IconThemeConfig::~IconThemeConfig()
{
}


void IconThemeConfig::iconThemeSelected(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QString theme = item->data(0, Qt::UserRole).toString();
    if (!theme.isEmpty())
    {
        XdgIcon::setThemeName(theme);

        // An hack to ensure that this widget also re loads it's own icons
        // from the selected icon theme.
        QIconLoader::instance()->setThemeName(QString());

        m_settings->setValue("icon_theme",  theme);
        m_settings->sync();
    }
}
