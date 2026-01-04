/* coded by Ketmar // Vampire Avalon (psyc://ketmar.no-ip.org/~Ketmar)
 * (c)DWTFYW
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */

#include "main.h"
#include "lxqttranslate.h"

#include <XdgIcon>
#include <LXQt/Application>
#include <LXQt/Settings>

///////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[])
{
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("koi8-r"));
    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("koi8-r"));

    LXQt::Application app(argc, argv);

    TRANSLATE_APP;

    //qDebug() << findDefaultTheme() << getCurrentTheme();

    SelectWnd *sw = new SelectWnd;
    sw->show();
    sw->setCurrent();
    return app.exec();
}
