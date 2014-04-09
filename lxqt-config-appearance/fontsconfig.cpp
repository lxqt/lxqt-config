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

    connect(ui->fontName, SIGNAL(currentFontChanged(QFont)), SLOT(updateQtFont()));
    connect(ui->fontStyle, SIGNAL(currentIndexChanged(int)), SLOT(updateQtFont()));
    connect(ui->fontSize, SIGNAL(valueChanged(int)), SLOT(updateQtFont()));

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

    QByteArray hintStyleStr = mSessionSettings.value("hint_style", "none").toByteArray();
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

    if(!hasOurFontConfig())
        updateFontConfig();
}

void FontsConfig::antialiasToggled(bool toggled)
{
    mSessionSettings.setValue("antialias", toggled);
    updateFontConfig();
}

void FontsConfig::dpiChanged(int value)
{
    mSessionSettings.setValue("dpi", value);
    updateFontConfig();
}

void FontsConfig::hintingToggled(bool toggled)
{
    mSessionSettings.setValue("hinting", toggled);
    updateFontConfig();
}

void FontsConfig::subpixelChanged(int index)
{
    mSessionSettings.setValue("subpixel", subpixelNames[index]);
    updateFontConfig();
}

void FontsConfig::hintStyleChanged(int index)
{
    mSessionSettings.setValue("hint_style", hintStyleNames[index]);
    updateFontConfig();
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

QString FontsConfig::configDir() const
{
    QString path = QString::fromLocal8Bit(qgetenv("XDG_CONFIG_HOME"));
    if(path.isEmpty())
        path = QDesktopServices::storageLocation(QDesktopServices::HomeLocation) % "/.config";
    path += "/fontconfig";
    return path;
}

bool FontsConfig::hasOurFontConfig() const
{
    bool ret = false;
    QString path = configDir() % "/fonts.conf";
    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
        // if the file already exsits, check if it's created by us
        QByteArray buffer = file.readAll();
        ret = buffer.contains("lxqt-config-appearance");
        file.close();
    }
    return ret;
}


// update fontconfig related settings
void FontsConfig::updateFontConfig()
{
    bool antialias = ui->antialias->isChecked();
    bool hinting = ui->hinting->isChecked();
    int subpixel = ui->subpixel->currentIndex();
    int hintStyle = ui->hintStyle->currentIndex();
    int dpi = ui->dpi->value();

    QString path = configDir();
    QDir().mkdir(path);
    path += "/fonts.conf";
    QFile file(path);

    // backup the old config file if it's not written by us
    if(!hasOurFontConfig() && file.exists())
	file.copy(path + ".bak");

    // References: https://wiki.archlinux.org/index.php/Font_Configuration
    if(file.open(QIODevice::WriteOnly))
    {
	QTextStream s(&file);
	s <<
	"<?xml version=\"1.0\"?>\n"
	"<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">\n"
	"<!-- created by lxqt-config-appearance **DO NOT EDIT** -->\n"
	"<fontconfig>\n"
	"  <match target=\"font\">\n"
	"    <edit name=\"antialias\" mode=\"assign\">\n"
	"      <bool>" << (antialias ? "true" : "false") << "</bool>\n"
	"    </edit>\n"
	"    <edit name=\"rgba\" mode=\"assign\">\n"
	"      <const>" << subpixelNames[subpixel] << "</const>\n"
	"    </edit>\n"
	"    <edit name=\"lcdfilter\" mode=\"assign\">\n"
	"      <const>lcddefault</const>\n"
	"    </edit>\n"
	"    <edit name=\"hinting\" mode=\"assign\">\n"
	"      <bool>" << (hinting ? "true" : "false") << "</bool>\n"
	"    </edit>\n"
	"    <edit name=\"hintstyle\" mode=\"assign\">\n"
	"      <const>hint" << hintStyleNames[hintStyle] << "</const>\n"
	"    </edit>\n"
	"  </match>\n"
	"  <match target=\"pattern\">\n"
	"    <edit name=\"dpi\" mode=\"assign\">\n"
	"      <double>" << dpi << "</double>\n"
	"    </edit>\n"
	"  </match>\n"
	"</fontconfig>";
	s.flush();
	file.close();
    }
}

/*
void FontsConfig::loadFontConfig()
{
    QString path = QString::fromLocal8Bit(qgetenv("XDG_CONFIG_HOME"));
    if(path.isEmpty())
        path = QDesktopServices::storageLocation(QDesktopServices::HomeLocation) % "/.config";
    path += "/fontconfig/fonts.conf";
    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray buffer = file.readAll();
	file.close();
	// check if this file is created by us
        // if the file already exsits, check if it's created by us
        if(!buffer.contains("lxqt-config-appearance"))
	{
	    
	}
    }
}
*/
