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
#include "monitorsettingsdialog.h"

static void killBackend()
{
    QProcess p;
    p.start("killall kscreen_backend_launcher");
    p.waitForFinished();
}

static void setBackend(const QString &backend)
{
    // Set new backend
    if(backend.size() == 0)
        qputenv("KSCREEN_BACKEND", QByteArray("XRandR"));
}


int main(int argc, char** argv)
{
    LxQt::SingleApplication app(argc, argv);
    // killBackend();
    // setBackend(QString::fromLatin1(qgetenv("KSCREEN_BACKEND")));

    MonitorSettingsDialog dlg;
    app.setActivationWindow(&dlg);
    dlg.setWindowIcon(QIcon::fromTheme("preferences-desktop-display"));
    dlg.show();

    int result = app.exec();
    // killBackend();
    return result;
}
