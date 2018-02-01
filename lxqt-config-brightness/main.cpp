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
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    // Command line options
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("LXQt Config Brightness"));
    const QString VERINFO = QStringLiteral(LXQT_CONFIG_VERSION
                                           "\nliblxqt   " LXQT_VERSION
                                           "\nQt        " QT_VERSION_STR);
    app.setApplicationVersion(VERINFO);
    QCommandLineOption increaseOption(QStringList() << "i" << "icrease",
            app.tr("Increase brightness."));
    QCommandLineOption decreaseOption(QStringList() << "d" << "decrease",
            app.tr("Decrease brightness."));
    QCommandLineOption setOption(QStringList() << "s" << "set",
            app.tr("Set brightness from 1 to 100."), "brightness");
    QCommandLineOption helpOption = parser.addHelpOption();
    parser.addOption(increaseOption);
    parser.addOption(decreaseOption);
    parser.addOption(setOption);
    parser.addOption(helpOption);
    parser.addVersionOption();

    parser.process(app);
    if( parser.isSet(increaseOption) || parser.isSet(decreaseOption) || parser.isSet(setOption) )
    {
        XRandrBrightness *brightness = new XRandrBrightness();
        const QList<MonitorInfo> monitors = brightness->getMonitorsInfo();
        QList<MonitorInfo> monitorsChanged;
        float sign = parser.isSet(decreaseOption)?-1.0:1.0;
        double brightness_value = parser.value(setOption).toFloat();
        brightness_value = qMin( qMax(brightness_value, 0.0), 100.0 ) / 100.0;
        if(!parser.value(setOption).isEmpty())
            sign = 0.0;
        for(MonitorInfo monitor : monitors)
        {
            if(monitor.isBacklightSupported() )
            {
                long backlight = ( monitor.backlight() + sign*(monitor.backlightMax()/50 + 1) )*qAbs(sign) + brightness_value*monitor.backlightMax();
                if(backlight<monitor.backlightMax() && backlight>0)
                {
                    monitor.setBacklight(backlight);
                    monitorsChanged.append(monitor);
                }
            }
            else
            {
                float brightness = (monitor.brightness() + 0.1*sign)*qAbs(sign) + brightness_value*2.0;
                if(brightness<2.0 && brightness>0.0)
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

