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

#include "fontsconfig.h"
#include "ui_fontsconfig.h"
#include <QTreeWidget>
#include <QDebug>
#include <QSettings>
#include <QFont>

#ifdef Q_WS_X11
extern void qt_x11_apply_settings_in_all_apps();
#endif

static const char* subpixelNames[] = {"none", "rgb", "bgr", "vrgb", "vbgr"};
static const char* hintStyleNames[] = {"none", "slight", "medium", "full"};

static inline QString sessionConfigName()
{
    QByteArray name = qgetenv("LXQT_SESSION_CONFIG");
    if(name.isEmpty())
        return QLatin1String("session");
    return QString::fromUtf8(name);
}

FontsConfig::FontsConfig(LxQt::Settings* settings, QSettings* qtSettings, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::FontsConfig),
    mSettings(settings),
    mQtSettings(qtSettings),
    mSessionSettings(sessionConfigName())
{
    ui->setupUi(this);
    mSessionSettings.beginGroup("Font");

    initControls();

    connect(ui->fontName, SIGNAL(currentFontChanged(QFont)), SLOT(fontChanged()));
    connect(ui->fontStyle, SIGNAL(currentIndexChanged(int)), SLOT(fontChanged()));
    connect(ui->fontSize, SIGNAL(valueChanged(int)), SLOT(fontChanged()));

    connect(ui->antialias, SIGNAL(toggled(bool)), SLOT(antialiasToggled(bool)));
    connect(ui->subpixel, SIGNAL(currentIndexChanged(int)), SLOT(subpixelChanged(int)));
    connect(ui->hinting, SIGNAL(toggled(bool)), SLOT(hintingToggled(bool)));
    connect(ui->hintStyle, SIGNAL(currentIndexChanged(int)), SLOT(hintStyleChanged(int)));
}


FontsConfig::~FontsConfig()
{
    mSessionSettings.sync();
    delete ui;
}


void FontsConfig::initControls()
{
    // read Qt style settings from Qt Trolltech.conf config
    mQtSettings->beginGroup(QLatin1String("Qt"));

    QString fontName = mQtSettings->value("font").toString();
    QFont font;
    font.fromString(fontName);
    ui->fontName->setCurrentFont(font);
    ui->fontSize->setValue(font.pointSize());
    int fontStyle = 0;
    if(font.bold())
      fontStyle = font.italic() ? 3 : 1;
    else if(font.italic())
      fontStyle = 2;
    ui->fontStyle->setCurrentIndex(fontStyle);

    mQtSettings->endGroup();

    // load LxQt session config
    bool antialias = mSessionSettings.value("antialias", true).toBool();
    ui->antialias->setChecked(antialias);

    QByteArray subpixelStr = mSessionSettings.value("subpixel", "none").toByteArray();
    int subpixel;
    for(subpixel = 0; subpixel < 5; ++subpixel)
    {
        if(subpixelStr == subpixelNames[subpixel])
            break;
    }
    if(subpixel < 5)
        ui->subpixel->setCurrentIndex(subpixel);
    
    bool hinting = mSessionSettings.value("hinting", true).toBool();
    ui->hinting->setChecked(hinting);

    QByteArray hintStyleStr = mSessionSettings.value("hinting_style", "none").toByteArray();
    int hintStyle;
    for(hintStyle = 0; hintStyle < 4; ++hintStyle)
    {
        if(hintStyleStr == hintStyleNames[hintStyle])
            break;
    }
    if(hintStyle < 4)
        ui->hintStyle->setCurrentIndex(hintStyle);

    int dpi = mSessionSettings.value("dpi", 96).toInt();
    ui->dpi->setValue(dpi);

    update();
}

void FontsConfig::antialiasToggled(bool toggled)
{
    mSessionSettings.setValue("antialias", toggled);
}

void FontsConfig::dpiChanged(int value)
{
    mSessionSettings.setValue("dpi", value);
}

void FontsConfig::hintingToggled(bool toggled)
{
    mSessionSettings.setValue("hinting", toggled);
}

void FontsConfig::subpixelChanged(int index)
{
    mSessionSettings.setValue("subpixel", subpixelNames[index]);
}

void FontsConfig::hintStyleChanged(int index)
{
    mSessionSettings.setValue("hinting_style", hintStyleNames[index]);
}

void FontsConfig::fontChanged()
{
    // FIXME: the change does not apply to some currently running Qt programs.
    // FIXME: does not work with KDE apps
    // TODO: also write the config values to GTK+ config files (gtk-2.0.rc and gtk3/settings.ini)
    // FIXME: the selected font does not apply to our own application. Why?

    QFont font = ui->fontName->currentFont();
    int size = ui->fontSize->value();
    bool bold = false;
    bool italic = false;
    switch(ui->fontStyle->currentIndex())
    {
        case 1:
            bold = true;
            break;
        case 2:
            italic = true;
            break;
        case 3:
          bold = italic = true;
    }

    font.setPointSize(size);
    font.setBold(bold);
    font.setItalic(italic);

    mQtSettings->beginGroup(QLatin1String("Qt"));
    mQtSettings->setValue("font", font.toString());
    mQtSettings->endGroup();
    mQtSettings->sync();

#ifdef Q_WS_X11
    qt_x11_apply_settings_in_all_apps();
#endif

    update();
}
