/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 *
 * Copyright: 2020 LXQt team
 * Authors:
 *   Pedro L. Lucas <selairi@gmail.com>
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


#include <LXQt/SingleApplication>
#include <QCommandLineParser>

#include "Daemon.h"



int main(int argc, char *argv[])
{
    LXQt::SingleApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("LXQt Screen Daemon"));
    const QString VERINFO = QSL(LXQT_CONFIG_VERSION \
                            "\nliblxqt   " LXQT_VERSION \
                            "\nQt        " QT_VERSION_STR);
    a.setApplicationVersion(VERINFO);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(a);

    Daemon daemon;

    return a.exec();
}