/*
    Copyright (C) 2013-2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <LXQt/SingleApplication>
#include <LXQt/ConfigDialog>
#include <LXQt/ConfigDialogCmdLineOptions>
#include <LXQt/Settings>
#include <QCommandLineParser>
#include "mouseconfig.h"
#include "keyboardconfig.h"
#include "../liblxqt-config-cursor/selectwnd.h"
#include "keyboardlayoutconfig.h"
#include "touchpadconfig.h"
#include "touchpaddevice.h"

int main(int argc, char** argv) {
    LXQt::SingleApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    QCommandLineParser parser;
    LXQt::ConfigDialogCmdLineOptions dlgOptions;
    parser.setApplicationDescription(QStringLiteral("LXQt Config Input"));
    const QString VERINFO = QStringLiteral(LXQT_CONFIG_VERSION
                                           "\n\nliblxqt:   " LXQT_VERSION
                                           "\nQt:        " QT_VERSION_STR);
    app.setApplicationVersion(VERINFO);

    dlgOptions.setCommandLine(&parser);
    QCommandLineOption loadOption("load-touchpad",
            app.tr("Load last touchpad settings."));
    parser.addOption(loadOption);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);
    dlgOptions.process(parser);

    QByteArray configName = qgetenv("LXQT_SESSION_CONFIG");
    if(configName.isEmpty())
      configName = "session";
    LXQt::Settings settings(configName);

    bool loadLastTouchpadSettings = parser.isSet(loadOption);
    if (loadLastTouchpadSettings) {
        TouchpadDevice::loadSettings(&settings);
        return 0;
    }

    LXQt::ConfigDialog dlg(QObject::tr("Keyboard and Mouse Settings"), &settings);
    app.setActivationWindow(&dlg);

    LXQt::Settings qtSettings("lxqt");
    MouseConfig* mouseConfig = new MouseConfig(&settings, &qtSettings, &dlg);
    dlg.addPage(mouseConfig, QObject::tr("Mouse"), "input-mouse");
    QObject::connect(&dlg, SIGNAL(reset()), mouseConfig, SLOT(reset()));

    SelectWnd* cursorConfig = new SelectWnd(&settings, &dlg);
    cursorConfig->setCurrent();
    dlg.addPage(cursorConfig, QObject::tr("Cursor"), "preferences-desktop-theme");

    KeyboardConfig* keyboardConfig = new KeyboardConfig(&settings, &qtSettings, &dlg);
    dlg.addPage(keyboardConfig, QObject::tr("Keyboard"), "input-keyboard");
    QObject::connect(&dlg, SIGNAL(reset()), keyboardConfig, SLOT(reset()));

    KeyboardLayoutConfig* keyboardLayoutConfig = new KeyboardLayoutConfig(&settings, &dlg);
    dlg.addPage(keyboardLayoutConfig, QObject::tr("Keyboard Layout"), "input-keyboard");
    QObject::connect(&dlg, SIGNAL(reset()), keyboardLayoutConfig, SLOT(reset()));

    TouchpadConfig* touchpadConfig = new TouchpadConfig(&settings, &dlg);
    dlg.addPage(touchpadConfig, QObject::tr("Touchpad"), "input-tablet");
    QObject::connect(&dlg, &LXQt::ConfigDialog::reset,
                     touchpadConfig, &TouchpadConfig::reset);

    dlg.setWindowIcon(QIcon::fromTheme("input-keyboard"));

    const QString initialPage = dlgOptions.page();
    if (!initialPage.isEmpty())
        dlg.showPage(initialPage);

    dlg.exec();
    return 0;
}
