/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt-project.org/
 *
 * Copyright: 2022 LXQt team
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

#include "gtkconfig.h"
#include "ui_gtkconfig.h"
#include <QDebug>

GTKConfig::GTKConfig(LXQt::Settings *configAppearanceSettings, ConfigOtherToolKits *configOtherToolKits, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::GTKConfig)
{
    mConfigAppearanceSettings = configAppearanceSettings;
    mConfigOtherToolKits = configOtherToolKits;
    ui->setupUi(this);

    initControls();

    connect(ui->advancedOptionsGroupBox, &QGroupBox::toggled, this, &GTKConfig::showAdvancedOptions);
    connect(ui->advancedOptionsGroupBox, &QGroupBox::clicked, this, &GTKConfig::settingsChanged);
    connect(ui->gtk2ComboBox, QOverload<int>::of(&QComboBox::activated), this, &GTKConfig::settingsChanged);
    connect(ui->gtk3ComboBox, QOverload<int>::of(&QComboBox::activated), this, &GTKConfig::settingsChanged);
}

GTKConfig::~GTKConfig()
{
    delete ui;
}

void GTKConfig::initControls()
{

    // Fill themes
    QStringList gtk2Themes = mConfigOtherToolKits->getGTKThemes(QStringLiteral("2.0"));
    QStringList gtk3Themes = mConfigOtherToolKits->getGTKThemes(QStringLiteral("3.*"));

    if(!mConfigAppearanceSettings->contains(QStringLiteral("ControlGTKThemeEnabled")))
        mConfigAppearanceSettings->setValue(QStringLiteral("ControlGTKThemeEnabled"), false);
    bool controlGTKThemeEnabled = mConfigAppearanceSettings->value(QStringLiteral("ControlGTKThemeEnabled")).toBool();

    showAdvancedOptions(controlGTKThemeEnabled);
    ui->advancedOptionsGroupBox->setChecked(controlGTKThemeEnabled);


    // Fill GTK themes
    ui->gtk2ComboBox->addItems(gtk2Themes);
    ui->gtk3ComboBox->addItems(gtk3Themes);

    ui->gtk2ComboBox->setCurrentText(mConfigOtherToolKits->getGTKThemeFromRCFile(QStringLiteral("2.0")));
    ui->gtk3ComboBox->setCurrentText(mConfigOtherToolKits->getGTKThemeFromRCFile(QStringLiteral("3.0")));

    update();
}

void GTKConfig::applyGTKStyle()
{
    if (ui->advancedOptionsGroupBox->isChecked())
    {
        // GTK3
        QString themeName = ui->gtk3ComboBox->currentText();
        mConfigOtherToolKits->setGTKConfig(QStringLiteral("3.0"), themeName);
        if(QGuiApplication::platformName() == QStringLiteral("wayland"))
            mConfigOtherToolKits->setGsettingsConfig(QStringLiteral("3.0"), themeName);
        // GTK2
        themeName = ui->gtk2ComboBox->currentText();
        mConfigOtherToolKits->setGTKConfig(QStringLiteral("2.0"), themeName);
        // Update xsettingsd
        mConfigOtherToolKits->setXSettingsConfig();
    }
}

void GTKConfig::showAdvancedOptions(bool on)
{
    ui->uniformThemeLabel->setVisible(on);
    if (mConfigAppearanceSettings->value(QStringLiteral("ControlGTKThemeEnabled")).toBool() != on)
        mConfigAppearanceSettings->setValue(QStringLiteral("ControlGTKThemeEnabled"), on);
    if (on)
        mConfigOtherToolKits->startXsettingsd();
}
