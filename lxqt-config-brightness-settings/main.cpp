/*
    Copyright (C) 2016  P.L. Lucas <selairi@gmail.com>
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
    
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "xrandrbrightness.h"
#include <QDebug>
#include <LXQt/SingleApplication>
#include "brightnesssettings.h"

int main(int argn, char* argv[])
{
    LXQt::SingleApplication app(argn, argv);
    /*
     * XRandrBrightness *brightness = new XRandrBrightness();
     * if(brightness->isSupported())
     * {
     *     qDebug() << "Brightness" << brightness->brightness();
     *     qDebug() << "Max. brightness" << brightness->brightnessMax();
     *     brightness->setPercentBrightness(0.04);
     * }
     * else
     * {
     *     qDebug("Brightness control is not supported");
     * }
     */

    BrightnessSettings *brightnessSettings = new BrightnessSettings();
    brightnessSettings->setWindowIcon(QIcon(ICON_DIR "/brightnesssettings.svg"));
    brightnessSettings->show();

    return app.exec();
}

