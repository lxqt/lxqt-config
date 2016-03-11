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
#include <QCommandLineParser>
#include "brightnesssettings.h"

int main(int argn, char* argv[])
{
    LXQt::SingleApplication app(argn, argv);
     
    // Command line options
    QCommandLineParser parser;
    QCommandLineOption increaseOption(QStringList() << "i" << "icrease",
            app.tr("Increase brightness."));
    parser.addOption(increaseOption);
    QCommandLineOption decreaseOption(QStringList() << "d" << "decrease",
            app.tr("Decrease brightness."));
    parser.addOption(decreaseOption);
    QCommandLineOption helpOption = parser.addHelpOption();
    parser.addOption(increaseOption);
    parser.addOption(decreaseOption);
    parser.addOption(helpOption);

    parser.process(app);
    if( parser.isSet(increaseOption) || parser.isSet(decreaseOption) )
    {
        XRandrBrightness *brightness = new XRandrBrightness();
        QList<MonitorInfo> monitors = brightness->getMonitorsInfo();
        QList<MonitorInfo> monitorsChanged;
        float sign = parser.isSet(decreaseOption)?-1.0:1.0;
        foreach(MonitorInfo monitor, monitors)
        {
            
            if( monitor.isBacklightSupported() )
            {
                long backlight = monitor.backlight() + sign*(monitor.backlightMax()/30 + 1);
                if(backlight<monitor.backlightMax())
                {
                    monitor.setBacklight(backlight);
                    monitorsChanged.append(monitor);
                }
            }
            else
            {
                float brightness = monitor.brightness() + 0.1*sign;
                if(brightness<2.0)
                {
                    monitor.setBrightness(brightness);
                    monitorsChanged.append(monitor);
                }
            }
        }
        brightness->setMonitorsSettings(monitorsChanged);
        return 0;
    }

    BrightnessSettings *brightnessSettings = new BrightnessSettings();
    brightnessSettings->setWindowIcon(QIcon(ICON_DIR "/brightnesssettings.svg"));
    brightnessSettings->show();

    return app.exec();
}

