/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt-project.org
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

#include "mainwindow.h"

#include <LXQt/SingleApplication>

#include <QCommandLineParser>
#include <QSettings>

int main(int argc, char **argv)
{
    LXQt::SingleApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("lxqt"));
    app.setApplicationName(QStringLiteral("lxqt-config"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("LXQt Config"));
    const QString VERINFO = QStringLiteral(LXQT_CONFIG_VERSION
                                           "\nliblxqt   " LXQT_VERSION
                                           "\nQt        " QT_VERSION_STR);
    app.setApplicationVersion(VERINFO);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    // ensure that we use lxqt-config.menu file.
    qputenv("XDG_MENU_PREFIX", "lxqt-");

    LXQtConfig::MainWindow w;
    app.setActivationWindow(&w);
    QSize s = QSettings{}.value(QStringLiteral("size")).toSize();
    if (!s.isEmpty())
        w.resize(s);
    w.show();

    int ret = app.exec();

    if (s != w.size())
        QSettings{}.setValue(QStringLiteral("size"), w.size());

    return ret;
}
