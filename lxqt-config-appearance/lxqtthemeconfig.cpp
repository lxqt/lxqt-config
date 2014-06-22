/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2012 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
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

#include "lxqtthemeconfig.h"
#include "ui_lxqtthemeconfig.h"
#include <QTreeWidget>
#include <QDebug>
#include <QProcess>

LxQtThemeConfig::LxQtThemeConfig(LxQt::Settings *settings, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LxQtThemeConfig),
    mSettings(settings)
{
    ui->setupUi(this);

    connect(ui->lxqtThemeList, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(lxqtThemeSelected(QTreeWidgetItem*,int)));


    QList<LxQt::LxQtTheme> themes = LxQt::LxQtTheme::allThemes();
    foreach(LxQt::LxQtTheme theme, themes)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(theme.name()));
        if (!theme.previewImage().isEmpty())
        {
            item->setIcon(0, QIcon(theme.previewImage()));
        }
        item->setSizeHint(0, QSize(42,42)); // make icons non-cropped
        item->setData(0, Qt::UserRole, theme.name());
        ui->lxqtThemeList->addTopLevelItem(item);
    }

    initControls();
}


LxQtThemeConfig::~LxQtThemeConfig()
{
    delete ui;
}


void LxQtThemeConfig::initControls()
{
    QString currentTheme = mSettings->value("theme").toString();

    QTreeWidgetItemIterator it(ui->lxqtThemeList);
    while (*it) {
        if ((*it)->data(0, Qt::UserRole).toString() == currentTheme)
        {
            ui->lxqtThemeList->setCurrentItem((*it));
            break;
        }
        ++it;
    }

    update();
}


void LxQtThemeConfig::lxqtThemeSelected(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    if (!item)
        return;

    QVariant themeName = item->data(0, Qt::UserRole);
    mSettings->setValue("theme", themeName);

    LxQt::LxQtTheme theme(themeName.toString());
    if(theme.isValid()) {
		QString wallpaper = theme.desktopBackground();
		if(!wallpaper.isEmpty()) {
			// call pcmanfm-qt to update wallpaper
			QProcess process;
			QStringList args;
			args << "--set-wallpaper" << wallpaper;
			process.start("pcmanfm-qt", args, QIODevice::NotOpen);
			process.waitForFinished();
		}
	}
}
