/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
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

#include <lxqt/lxqtapplication.h>

#include <qtxdg/xdgicon.h>
#include <lxqt/lxqtsettings.h>
#include <lxqt/lxqtconfigdialog.h>
#include "iconthemeconfig.h"
#include "lxqttranslate.h"
#include "lxqtthemeconfig.h"
#include "styleconfig.h"

int main (int argc, char **argv)
{
    LxQt::Application app(argc, argv);
    TRANSLATE_APP;

    LxQt::Settings* settings = new LxQt::Settings("lxqt");
    LxQt::ConfigDialog* dialog = new LxQt::ConfigDialog(QObject::tr("LxQt Appearance Configuration"), settings);

    QSettings qtSettings(QLatin1String("Trolltech"));
    StyleConfig* stylePage = new StyleConfig(&qtSettings);
    dialog->addPage(stylePage, QObject::tr("Widget Style"), QStringList() << "preferences-desktop-theme" << "preferences-desktop");
    QObject::connect(dialog, SIGNAL(reset()), stylePage, SLOT(initControls()));

    IconThemeConfig* iconPage = new IconThemeConfig(settings);
    dialog->addPage(iconPage, QObject::tr("Icons Theme"), QStringList() << "preferences-desktop-icons" << "preferences-desktop");
    QObject::connect(dialog, SIGNAL(reset()), iconPage, SLOT(initControls()));

    LxQtThemeConfig* themePage = new LxQtThemeConfig(settings);
    dialog->addPage(themePage, QObject::tr("LxQt Theme"), QStringList() << "preferences-desktop-color" << "preferences-desktop");
    QObject::connect(dialog, SIGNAL(reset()), themePage, SLOT(initControls()));

    dialog->show();

    return app.exec();
}

