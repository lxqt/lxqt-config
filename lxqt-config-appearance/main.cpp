/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
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

#include <LXQt/Application>
#include <LXQt/Translator>

#include <XdgIcon>
#include <LXQt/Settings>
#include <LXQt/ConfigDialog>
#include "iconthemeconfig.h"
#include "lxqtthemeconfig.h"
#include "styleconfig.h"
#include "fontsconfig.h"

#include "../liblxqt-config-cursor/selectwnd.h"

int main (int argc, char **argv)
{
    LxQt::Application app(argc, argv);
    LxQt::Translator::translateApplication(QLatin1String(PROJECT_NAME));
#if (QT_VERSION < QT_VERSION_CHECK(5, 1, 0))
    LxQt::Translator::translateLibrary(QLatin1String("lxqt-config-cursor"));
#endif

    LxQt::Settings* settings = new LxQt::Settings("lxqt");
    LxQt::ConfigDialog* dialog = new LxQt::ConfigDialog(QObject::tr("LXQt Appearance Configuration"), settings);

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QSettings& qtSettings = *settings; // use lxqt config file for Qt settings in Qt5.
#else
    QSettings qtSettings(QLatin1String("Trolltech")); // Qt 4.x only (Qt 5 deprecated Trolltech.conf)
#endif
    StyleConfig* stylePage = new StyleConfig(settings, &qtSettings, dialog);
    dialog->addPage(stylePage, QObject::tr("Widget Style"), QStringList() << "preferences-desktop-theme" << "preferences-desktop");
    QObject::connect(dialog, SIGNAL(reset()), stylePage, SLOT(initControls()));

    IconThemeConfig* iconPage = new IconThemeConfig(settings, dialog);
    dialog->addPage(iconPage, QObject::tr("Icons Theme"), QStringList() << "preferences-desktop-icons" << "preferences-desktop");
    QObject::connect(dialog, SIGNAL(reset()), iconPage, SLOT(initControls()));

    LxQtThemeConfig* themePage = new LxQtThemeConfig(settings, dialog);
    dialog->addPage(themePage, QObject::tr("LXQt Theme"), QStringList() << "preferences-desktop-color" << "preferences-desktop");
    QObject::connect(dialog, SIGNAL(reset()), themePage, SLOT(initControls()));

    FontsConfig* fontsPage = new FontsConfig(settings, &qtSettings, dialog);
    dialog->addPage(fontsPage, QObject::tr("Font"), QStringList() << "preferences-desktop-font" << "preferences-desktop");
    QObject::connect(dialog, SIGNAL(reset()), fontsPage, SLOT(initControls()));

    SelectWnd* cursorPage = new SelectWnd(settings, dialog);
    cursorPage->setCurrent();
    dialog->addPage(cursorPage, QObject::tr("Cursor"), QStringList() << "input-mouse" << "preferences-desktop");

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();

    return app.exec();
}

