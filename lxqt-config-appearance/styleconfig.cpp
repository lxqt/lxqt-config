/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org/
 *
 * Copyright: 2014 LXQt team
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
#include <QToolBar>
#include <QSettings>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaEnum>
#include <QToolBar>

#ifdef Q_WS_X11
extern void qt_x11_apply_settings_in_all_apps();
#endif

StyleConfig::StyleConfig(LXQt::Settings* settings, QSettings* qtSettings, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::StyleConfig),
    mQtSettings(qtSettings),
    mSettings(settings)
{
    ui->setupUi(this);

    connect(ui->styleList, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(styleSelected(QTreeWidgetItem*,int)));

    const auto keys = QStyleFactory::keys();
    for(const QString& name : keys)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(name));
        ui->styleList->addTopLevelItem(item);
    }

    initControls();

    connect(ui->toolButtonStyle, SIGNAL(currentIndexChanged(int)), SLOT(toolButtonStyleSelected(int)));
    connect(ui->singleClickActivate, SIGNAL(toggled(bool)), SLOT(singleClickActivateToggled(bool)));
}


StyleConfig::~StyleConfig()
{
    delete ui;
}


void StyleConfig::initControls()
{
    // read Qt style settings from Qt Trolltech.conf config
    mQtSettings->beginGroup(QLatin1String("Qt"));
    QString currentTheme = mQtSettings->value("style").toString();
    mQtSettings->endGroup();

    QTreeWidgetItemIterator it(ui->styleList);
    while (*it) {
        if ((*it)->data(0, Qt::DisplayRole).toString() == currentTheme)
        {
            ui->styleList->setCurrentItem((*it));
            break;
        }
        ++it;
    }

    // read other widget related settings form LXQt settings.
    QByteArray tb_style = mSettings->value("tool_button_style").toByteArray();
    // convert toolbar style name to value
    QMetaEnum me = QToolBar::staticMetaObject.property(QToolBar::staticMetaObject.indexOfProperty("toolButtonStyle")).enumerator();
    int val = me.keyToValue(tb_style.constData());
    if(val == -1)
      val = Qt::ToolButtonTextBesideIcon;
    ui->toolButtonStyle->setCurrentIndex(val);

    // activate item views with single click
    ui->singleClickActivate->setChecked( mSettings->value("single_click_activate", false).toBool());

    update();
}


void StyleConfig::styleSelected(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    if (!item)
        return;
    QVariant themeName = item->data(0, Qt::DisplayRole);
    mQtSettings->beginGroup(QLatin1String("Qt"));
    mQtSettings->setValue("style", themeName);
    mQtSettings->endGroup();
    mQtSettings->sync();

#ifdef Q_WS_X11
    qt_x11_apply_settings_in_all_apps();
#endif
}

void StyleConfig::toolButtonStyleSelected(int index)
{
    // convert style value to string
    QMetaEnum me = QToolBar::staticMetaObject.property(QToolBar::staticMetaObject.indexOfProperty("toolButtonStyle")).enumerator();
    if(index == -1)
        index = Qt::ToolButtonTextBesideIcon;
    const char* str = me.valueToKey(index);
    if(str)
    {
        mSettings->setValue("tool_button_style", str);
	mSettings->sync();
    }
}

void StyleConfig::singleClickActivateToggled(bool toggled)
{
    mSettings->setValue("single_click_activate", toggled);
    mSettings->sync();
}

