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
    mConfigOtherToolKits = new ConfigOtherToolKits(settings, this);
    ui->setupUi(this);

    initControls();
    
    connect(ui->globalThemeComboBox, SIGNAL(activated(const QString &)), this, SLOT(globalThemeSelected(const QString &)));
    connect(ui->gtkComboBox, SIGNAL(activated(const QString &)), this, SLOT(gtkStyleSelected(const QString &)));
    connect(ui->qtComboBox, SIGNAL(activated(const QString &)), this, SLOT(qtStyleSelected(const QString &)));
    
    connect(ui->advancedOptionsGroupBox, SIGNAL(toggled(bool)), this, SLOT(showAdvancedOptions(bool)));
    connect(ui->advancedOptionsGroupBox, SIGNAL(toggled(bool)), this, SLOT(setAdvancedSettings(bool)));
    
    connect(ui->toolButtonStyle, SIGNAL(currentIndexChanged(int)), SLOT(toolButtonStyleSelected(int)));
    connect(ui->singleClickActivate, SIGNAL(toggled(bool)), SLOT(singleClickActivateToggled(bool)));
}


StyleConfig::~StyleConfig()
{
    delete ui;
}


void StyleConfig::initControls()
{

    // Fill global themes
    QStringList qtThemes = QStyleFactory::keys();
    QStringList gtk2Themes = mConfigOtherToolKits->getGTKThemes("2.0");
    QStringList gtk3Themes = mConfigOtherToolKits->getGTKThemes("3.0");
    QStringList globalThemeList;
    // Common themes
    for(QString qtTheme : qtThemes) {
        if( !globalThemeList.contains(qtTheme) &&  gtk3Themes.contains(qtTheme)  &&  gtk2Themes.contains(qtTheme) )
            globalThemeList.append(qtTheme);
    }
    if(qtThemes.contains("gtk2")) {
        // GTK2 and GTK3 themes. Qt style will be set to gtk2.
        for(QString theme : gtk2Themes) {
            if( !globalThemeList.contains(theme) &&  gtk3Themes.contains(theme) )
                globalThemeList.append(theme);
        }
    }
    
    ui->globalThemeComboBox->addItems( globalThemeList );
    mSettings->beginGroup(QLatin1String("Themes"));
    ui->globalThemeComboBox->setCurrentText(mSettings->value("GlobalThemeName").toString());
    if(!mSettings->contains("GlobalThemeEnabled"))
        mSettings->setValue("GlobalThemeEnabled", true);
    bool GlobalThemeEnabledd = mSettings->value("GlobalThemeEnabled").toBool();
    mSettings->endGroup();
    
    showAdvancedOptions(!GlobalThemeEnabledd);
    ui->advancedOptionsGroupBox->setChecked(!GlobalThemeEnabledd);

    // read other widget related settings from LXQt settings.
    QByteArray tb_style = mSettings->value("tool_button_style").toByteArray();
    // convert toolbar style name to value
    QMetaEnum me = QToolBar::staticMetaObject.property(QToolBar::staticMetaObject.indexOfProperty("toolButtonStyle")).enumerator();
    int val = me.keyToValue(tb_style.constData());
    if(val == -1)
      val = Qt::ToolButtonTextBesideIcon;
    ui->toolButtonStyle->setCurrentIndex(val);

    // activate item views with single click
    ui->singleClickActivate->setChecked( mSettings->value("single_click_activate", false).toBool());
    
    
    // Fill Qt themes
    ui->qtComboBox->addItems(qtThemes);
    
    // Fill GTK themes
    ui->gtkComboBox->addItems(gtk3Themes);
    
    mSettings->beginGroup(QLatin1String("Themes"));
    if(! mSettings->contains("GTK3ThemeName")) {
        // Set default values
        QString themeName = mConfigOtherToolKits->getGTKThemeFromRCFile("3.0");
        mSettings->setValue("GTK3ThemeName", themeName);
        mSettings->setValue("GTK2ThemeName", themeName);
    }
    if(! mSettings->contains("QtThemeName")) {
        // Set default values
        mSettings->endGroup();
        mQtSettings->beginGroup(QLatin1String("Qt"));
        QString styleName = mQtSettings->value("style").toString();
        mQtSettings->endGroup();
        mSettings->beginGroup(QLatin1String("Themes"));
        mSettings->setValue("QtThemeName", styleName);
    }
    ui->gtkComboBox->setCurrentText(mSettings->value("GTK3ThemeName").toString());
    ui->qtComboBox->setCurrentText(mSettings->value("QtThemeName").toString());
    mSettings->endGroup();

    update();
}

void StyleConfig::globalThemeSelected(const QString &themeName)
{
    mSettings->beginGroup(QLatin1String("Themes"));
    mSettings->setValue("GlobalThemeName", themeName);
    mSettings->endGroup();
    mSettings->sync();

    mQtSettings->beginGroup(QLatin1String("Qt"));
    mQtSettings->setValue("style", QStyleFactory::keys().contains(themeName) ? themeName : "gtk2");
    mQtSettings->endGroup();
    mQtSettings->sync();
    
#ifdef Q_WS_X11
    qt_x11_apply_settings_in_all_apps();
#endif
    
    mConfigOtherToolKits->setConfig();
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
        emit updateSettings();
    }
}

void StyleConfig::singleClickActivateToggled(bool toggled)
{
    mSettings->setValue("single_click_activate", toggled);
    mSettings->sync();
}

void StyleConfig::qtStyleSelected(const QString &themeName)
{
    mQtSettings->beginGroup(QLatin1String("Qt"));
    mQtSettings->setValue("style", themeName);
    mQtSettings->endGroup();
    mQtSettings->sync();
    mSettings->beginGroup(QLatin1String("Themes"));
    mSettings->setValue("QtThemeName", themeName);
    mSettings->endGroup();
    mSettings->sync();
}

void StyleConfig::gtkStyleSelected(const QString &themeName)
{
    mSettings->beginGroup(QLatin1String("Themes"));
    mSettings->setValue("GTK3ThemeName", themeName);
    mSettings->setValue("GTK2ThemeName", themeName);
    mSettings->endGroup();
    mSettings->sync();

    mConfigOtherToolKits->setConfig();
}

void StyleConfig::showAdvancedOptions(bool on)
{
    ui->globalThemeLabel->setEnabled(!on);
    ui->globalThemeComboBox->setEnabled(!on);
}

void StyleConfig::setAdvancedSettings(bool on)
{
    mSettings->beginGroup(QLatin1String("Themes"));
    mSettings->setValue("GlobalThemeEnabled", !on);
    mSettings->endGroup();
    mSettings->sync();
    if(!on) {
        // Use global theme
        mSettings->beginGroup(QLatin1String("Themes"));
        QString globalThemeName = mSettings->value("GlobalThemeName").toString();
        mSettings->endGroup();
        globalThemeSelected(globalThemeName);
        mConfigOtherToolKits->setConfig();
    } else {
        mSettings->beginGroup(QLatin1String("Themes"));
        QString gtk2ThemeName = mSettings->value("GTK2ThemeName").toString();
        QString gtk3ThemeName = mSettings->value("GTK3ThemeName").toString();
        QString qtThemeName = mSettings->value("QtThemeName").toString();
        mSettings->endGroup();
        gtkStyleSelected(gtk3ThemeName);
        qtStyleSelected(qtThemeName);
    }
}