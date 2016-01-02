/*
    Copyright (C) 2014  P.L. Lucas <selairi@gmail.com>

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
#include <LXQt/Settings>
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QCommandLineParser>
#include "monitorsettingsdialog.h"
#include <QCoreApplication>
#include "loadsettings.h"

static bool loadSettingsOk(int argc, char** argv)
{
    for(int i=0; i<argc; i++)
    {
        if(QString(argv[i]) == "-l")
            return true;
    }
    return false;
}

int main(int argc, char** argv)
{
    if( loadSettingsOk(argc, argv) )
    {
        // If -l option is provided, settings are loaded and app is closed.
        QCoreApplication app(argc, argv);
        LoadSettings load;
        return app.exec();
    }

    LXQt::SingleApplication app(argc, argv);

    // Command line options
    QCommandLineParser parser;
    QCommandLineOption loadOption(QStringList() << "l" << "loadlast",
            app.tr("Load last settings."));
    parser.addOption(loadOption);
    QCommandLineOption helpOption = parser.addHelpOption();
    parser.addOption(loadOption);
    parser.addOption(helpOption);

    //parser.process(app);
    //bool loadLastSettings = parser.isSet(loadOption);

    MonitorSettingsDialog dlg;
    app.setActivationWindow(&dlg);
    dlg.setWindowIcon(QIcon::fromTheme("preferences-desktop-display"));
    dlg.show();

    int ok = app.exec();
    
    system("killall kscreen_backend_launcher");
    
    return ok;

}
