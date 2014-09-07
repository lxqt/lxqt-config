/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * http://lxde.org/
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

#include "fontsconfig.h"
#include "ui_fontsconfig.h"
#include <QTreeWidget>
#include <QDebug>
#include <QSettings>
#include <QFont>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QTextStream>
#include <QStringBuilder>
#include <QDomDocument>

#ifdef Q_WS_X11
extern void qt_x11_apply_settings_in_all_apps();
#endif

static const char* subpixelNames[] = {"none", "rgb", "bgr", "vrgb", "vbgr"};
static const char* hintStyleNames[] = {"hintnone", "hintslight", "hintmedium", "hintfull"};

FontsConfig::FontsConfig(LxQt::Settings* settings, QSettings* qtSettings, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::FontsConfig),
    mSettings(settings),
    mQtSettings(qtSettings),
    mFontConfigFile()
{
    ui->setupUi(this);

    initControls();

    connect(ui->fontName, SIGNAL(currentFontChanged(QFont)), SLOT(updateQtFont()));
    connect(ui->fontStyle, SIGNAL(currentIndexChanged(int)), SLOT(updateQtFont()));
    connect(ui->fontSize, SIGNAL(valueChanged(int)), SLOT(updateQtFont()));

    connect(ui->antialias, SIGNAL(toggled(bool)), SLOT(antialiasToggled(bool)));
    connect(ui->subpixel, SIGNAL(currentIndexChanged(int)), SLOT(subpixelChanged(int)));
    connect(ui->hinting, SIGNAL(toggled(bool)), SLOT(hintingToggled(bool)));
    connect(ui->hintStyle, SIGNAL(currentIndexChanged(int)), SLOT(hintStyleChanged(int)));
    connect(ui->dpi, SIGNAL(valueChanged(int)), SLOT(dpiChanged(int)));
    connect(ui->autohint, SIGNAL(toggled(bool)), SLOT(autohintToggled(bool)));
}


FontsConfig::~FontsConfig()
{
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

    // load fontconfig config
    ui->antialias->setChecked(mFontConfigFile.antialias());
    ui->autohint->setChecked(mFontConfigFile.autohint());

    QByteArray subpixelStr = mFontConfigFile.subpixel();
    int subpixel;
    for(subpixel = 0; subpixel < 5; ++subpixel)
    {
        if(subpixelStr == subpixelNames[subpixel])
            break;
    }
    if(subpixel < 5)
        ui->subpixel->setCurrentIndex(subpixel);

    ui->hinting->setChecked(mFontConfigFile.hinting());

    QByteArray hintStyleStr = mFontConfigFile.hintStyle();
    int hintStyle;
    for(hintStyle = 0; hintStyle < 4; ++hintStyle)
    {
        if(hintStyleStr == hintStyleNames[hintStyle])
            break;
    }
    if(hintStyle < 4)
        ui->hintStyle->setCurrentIndex(hintStyle);

    int dpi = mFontConfigFile.dpi();
    ui->dpi->setValue(dpi);

    update();
}

void FontsConfig::antialiasToggled(bool toggled)
{
    mFontConfigFile.setAntialias(toggled);
}

void FontsConfig::dpiChanged(int value)
{
    mFontConfigFile.setDpi(value);
}

void FontsConfig::hintingToggled(bool toggled)
{
    mFontConfigFile.setHinting(toggled);
}

void FontsConfig::subpixelChanged(int index)
{
    mFontConfigFile.setSubpixel(subpixelNames[index]);
}

void FontsConfig::hintStyleChanged(int index)
{
    mFontConfigFile.setHintStyle(hintStyleNames[index]);
}

void FontsConfig::autohintToggled(bool toggled)
{
    mFontConfigFile.setAutohint(toggled);
}

void FontsConfig::updateQtFont()
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
