/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://lxde.org/
 *
 * Copyright: 2014 LxQt team
 * Authors:
 *   Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
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

#include "styleconfig.h"
#include "ui_styleconfig.h"
#include <QTreeWidget>
#include <QDebug>
#include <QStyleFactory>
#include <QSettings>

#ifdef Q_WS_X11
extern void qt_x11_apply_settings_in_all_apps();
#endif

StyleConfig::StyleConfig(QSettings* settings, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::StyleConfig),
    mSettings(settings)
{
    ui->setupUi(this);

    connect(ui->styleList, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(styleSelected(QTreeWidgetItem*,int)));

    Q_FOREACH(const QString& name, QStyleFactory::keys())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(name));
        ui->styleList->addTopLevelItem(item);
    }
    initControls();
}


StyleConfig::~StyleConfig()
{
    delete ui;
}


void StyleConfig::initControls()
{
    mSettings->beginGroup(QLatin1String("Qt"));
    QString currentTheme = mSettings->value("style").toString();
    qDebug() << currentTheme;
    mSettings->endGroup();

    QTreeWidgetItemIterator it(ui->styleList);
    while (*it) {
        if ((*it)->data(0, Qt::DisplayRole).toString() == currentTheme)
        {
            ui->styleList->setCurrentItem((*it));
            break;
        }
        ++it;
    }
    update();
}


void StyleConfig::styleSelected(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    if (!item)
        return;
    QVariant themeName = item->data(0, Qt::DisplayRole);
    mSettings->beginGroup(QLatin1String("Qt"));
    mSettings->setValue("style", themeName);
    mSettings->endGroup();
    mSettings->sync();

#ifdef Q_WS_X11
    qt_x11_apply_settings_in_all_apps();
#endif
}
