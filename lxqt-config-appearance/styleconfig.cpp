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
#include <LXQt/lxqtplatform.h>


#ifdef Q_WS_X11
extern void qt_x11_apply_settings_in_all_apps();
#endif

StyleConfig::StyleConfig(LXQt::Settings* settings, QSettings* qtSettings, LXQt::Settings *configAppearanceSettings, ConfigOtherToolKits *configOtherToolKits, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::StyleConfig),
    mQtSettings(qtSettings),
    mSettings(settings)
{
    mConfigAppearanceSettings = configAppearanceSettings;
    mConfigOtherToolKits = configOtherToolKits;
    ui->setupUi(this);

    initControls();

    connect(ui->advancedOptionsGroupBox, &QGroupBox::toggled, this, &StyleConfig::showAdvancedOptions);

    connect(ui->qtComboBox, QOverload<int>::of(&QComboBox::activated), this, &StyleConfig::settingsChanged);
    connect(ui->advancedOptionsGroupBox, &QGroupBox::clicked, this, &StyleConfig::settingsChanged);
    connect(ui->gtk2ComboBox, QOverload<int>::of(&QComboBox::activated), this, &StyleConfig::settingsChanged);
    connect(ui->gtk3ComboBox, QOverload<int>::of(&QComboBox::activated), this, &StyleConfig::settingsChanged);
    connect(ui->toolButtonStyle, QOverload<int>::of(&QComboBox::activated), this, &StyleConfig::settingsChanged);
    connect(ui->singleClickActivate, &QAbstractButton::clicked, this, &StyleConfig::settingsChanged);

    connect(ui->winColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->baseColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->highlightColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->windowTextColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->viewTextColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->highlightedTextColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->linkColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->linkVisitedColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);

    connect(ui->defaultPaletteBtn, &QAbstractButton::clicked, [this] {
        QColor winColor(239, 239, 239);
        QPalette defaultPalette(winColor);
        ui->winColorLabel->setColor(winColor, true);
        ui->baseColorLabel->setColor(defaultPalette.color(QPalette::Active,QPalette::Base), true);
        // Qt's default highlight color may be different from that of Fusion
        ui->highlightColorLabel->setColor(QColor(60, 140, 230), true);
        ui->windowTextColorLabel->setColor(defaultPalette.color(QPalette::Active,QPalette::WindowText), true);
        ui->viewTextColorLabel->setColor(defaultPalette.color(QPalette::Active,QPalette::Text), true);
        ui->highlightedTextColorLabel->setColor(defaultPalette.color(QPalette::Active,QPalette::HighlightedText), true);
        ui->linkColorLabel->setColor(defaultPalette.color(QPalette::Active,QPalette::Link), true);
        ui->linkVisitedColorLabel->setColor(defaultPalette.color(QPalette::Active,QPalette::LinkVisited), true);
    });
}


StyleConfig::~StyleConfig()
{
    delete ui;
}


void StyleConfig::initControls()
{

    // Fill global themes
    QStringList qtThemes = QStyleFactory::keys();
    QStringList gtk2Themes = mConfigOtherToolKits->getGTKThemes(QStringLiteral("2.0"));
    QStringList gtk3Themes = mConfigOtherToolKits->getGTKThemes(QStringLiteral("3.*"));

    if(!mConfigAppearanceSettings->contains(QStringLiteral("ControlGTKThemeEnabled")))
        mConfigAppearanceSettings->setValue(QStringLiteral("ControlGTKThemeEnabled"), false);
    bool controlGTKThemeEnabled = mConfigAppearanceSettings->value(QStringLiteral("ControlGTKThemeEnabled")).toBool();

    showAdvancedOptions(controlGTKThemeEnabled);
    ui->advancedOptionsGroupBox->setChecked(controlGTKThemeEnabled);

    // read other widget related settings from LXQt settings.
    QByteArray tb_style = mSettings->value(QStringLiteral("tool_button_style")).toByteArray();
    // convert toolbar style name to value
    QMetaEnum me = QToolBar::staticMetaObject.property(QToolBar::staticMetaObject.indexOfProperty("toolButtonStyle")).enumerator();
    int val = me.keyToValue(tb_style.constData());
    if(val == -1)
      val = Qt::ToolButtonTextBesideIcon;
    ui->toolButtonStyle->setCurrentIndex(val);

    // activate item views with single click
    ui->singleClickActivate->setChecked( mSettings->value(QStringLiteral("single_click_activate"), false).toBool());

    // Fill Qt themes
    ui->qtComboBox->clear();
    ui->qtComboBox->addItems(qtThemes);

    // Fill GTK themes
    ui->gtk2ComboBox->addItems(gtk2Themes);
    ui->gtk3ComboBox->addItems(gtk3Themes);

    ui->gtk2ComboBox->setCurrentText(mConfigOtherToolKits->getGTKThemeFromRCFile(QStringLiteral("2.0")));
    ui->gtk3ComboBox->setCurrentText(mConfigOtherToolKits->getGTKThemeFromRCFile(QStringLiteral("3.0")));

    // Qt style
    mSettings->beginGroup(QLatin1String("Qt"));
    ui->qtComboBox->setCurrentText(mSettings->value(QStringLiteral("style")).toString());
    mSettings->endGroup();

    // palette
    mSettings->beginGroup(QLatin1String("Palette"));
    QColor color;
    color.setNamedColor(mSettings->value(QStringLiteral("window_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Window);
    ui->winColorLabel->setColor(color);

    color.setNamedColor(mSettings->value(QStringLiteral("base_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Base);
    ui->baseColorLabel->setColor(color);

    color.setNamedColor(mSettings->value(QStringLiteral("highlight_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Highlight);
    ui->highlightColorLabel->setColor(color);

    color.setNamedColor(mSettings->value(QStringLiteral("window_text_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::WindowText);
    ui->windowTextColorLabel->setColor(color);

    color.setNamedColor(mSettings->value(QStringLiteral("text_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Text);
    ui->viewTextColorLabel->setColor(color);

    color.setNamedColor(mSettings->value(QStringLiteral("highlighted_text_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::HighlightedText);
    ui->highlightedTextColorLabel->setColor(color);

    color.setNamedColor(mSettings->value(QStringLiteral("link_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Link);
    ui->linkColorLabel->setColor(color);

    color.setNamedColor(mSettings->value(QStringLiteral("link_visited_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::LinkVisited);
    ui->linkVisitedColorLabel->setColor(color);

    mSettings->endGroup();

    update();
}

void StyleConfig::applyStyle()
{
    // Qt style
    QString themeName = ui->qtComboBox->currentText();;
    mQtSettings->beginGroup(QLatin1String("Qt"));
    if(mQtSettings->value(QStringLiteral("style")).toString() != themeName)
        mQtSettings->setValue(QStringLiteral("style"), themeName);
    mQtSettings->endGroup();

    // palette
    mSettings->beginGroup(QLatin1String("Palette"));
    QColor color = ui->winColorLabel->getColor();
    QColor oldColor;
    oldColor.setNamedColor(mQtSettings->value(QStringLiteral("window_color")).toString());
    if (color != oldColor)
        mQtSettings->setValue(QStringLiteral("window_color"), color.name());

    color = ui->baseColorLabel->getColor();
    oldColor.setNamedColor(mQtSettings->value(QStringLiteral("base_color")).toString());
    if (color != oldColor)
        mQtSettings->setValue(QStringLiteral("base_color"), color.name());

    color = ui->highlightColorLabel->getColor();
    oldColor.setNamedColor(mQtSettings->value(QStringLiteral("highlight_color")).toString());
    if (color != oldColor)
        mQtSettings->setValue(QStringLiteral("highlight_color"), color.name());

    color = ui->windowTextColorLabel->getColor();
    oldColor.setNamedColor(mQtSettings->value(QStringLiteral("window_text_color")).toString());
    if (color != oldColor)
        mQtSettings->setValue(QStringLiteral("window_text_color"), color.name());

    color = ui->viewTextColorLabel->getColor();
    oldColor.setNamedColor(mQtSettings->value(QStringLiteral("text_color")).toString());
    if (color != oldColor)
        mQtSettings->setValue(QStringLiteral("text_color"), color.name());

    color = ui->highlightedTextColorLabel->getColor();
    oldColor.setNamedColor(mQtSettings->value(QStringLiteral("highlighted_text_color")).toString());
    if (color != oldColor)
        mQtSettings->setValue(QStringLiteral("highlighted_text_color"), color.name());

    color = ui->linkColorLabel->getColor();
    oldColor.setNamedColor(mQtSettings->value(QStringLiteral("link_color")).toString());
    if (color != oldColor)
        mQtSettings->setValue(QStringLiteral("link_color"), color.name());

    color = ui->linkVisitedColorLabel->getColor();
    oldColor.setNamedColor(mQtSettings->value(QStringLiteral("link_visited_color")).toString());
    if (color != oldColor)
        mQtSettings->setValue(QStringLiteral("link_visited_color"), color.name());

    mQtSettings->endGroup();

    // single click setting
    if(mSettings->value(QStringLiteral("single_click_activate")).toBool() !=  ui->singleClickActivate->isChecked()) {
        mSettings->setValue(QStringLiteral("single_click_activate"), ui->singleClickActivate->isChecked());
    }

   // tool button style
   int index = ui->toolButtonStyle->currentIndex();
    // convert style value to string
    QMetaEnum me = QToolBar::staticMetaObject.property(QToolBar::staticMetaObject.indexOfProperty("toolButtonStyle")).enumerator();
    if(index == -1)
        index = Qt::ToolButtonTextBesideIcon;
    const char* str = me.valueToKey(index);
    if(str && mSettings->value(QStringLiteral("tool_button_style")) != QString::fromUtf8(str))
    {
        mSettings->setValue(QStringLiteral("tool_button_style"), QString::fromUtf8(str));
        mSettings->sync();
        mConfigOtherToolKits->setConfig();
    }

    if (ui->advancedOptionsGroupBox->isChecked())
    {
        // GTK3
        themeName = ui->gtk3ComboBox->currentText();
        mConfigOtherToolKits->setGTKConfig(QStringLiteral("3.0"), themeName);
        LXQt::Platform::PLATFORM platform = LXQt::Platform::getPlatform();
        if(platform == LXQt::Platform::WAYLAND)
            mConfigOtherToolKits->setGsettingsConfig(QStringLiteral("3.0"), themeName);
        // GTK2
        themeName = ui->gtk2ComboBox->currentText();
        mConfigOtherToolKits->setGTKConfig(QStringLiteral("2.0"), themeName);
        // Update xsettingsd
        mConfigOtherToolKits->setXSettingsConfig();
    }
}

void StyleConfig::showAdvancedOptions(bool on)
{
    ui->uniformThemeLabel->setVisible(on);
    mConfigAppearanceSettings->setValue(QStringLiteral("ControlGTKThemeEnabled"), on);
}
