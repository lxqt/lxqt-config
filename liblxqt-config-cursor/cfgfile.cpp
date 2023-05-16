/* coded by Ketmar // Vampire Avalon (psyc://ketmar.no-ip.org/~Ketmar)
 * (c)DWTFYW
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */
#include "cfgfile.h"

#include <QDir>
#include <QFile>
#include <QStringList>
#include <QTextStream>

QMultiMap<QString, QString> loadCfgFile(const QString &fname, bool forceLoCase)
{
    QMultiMap<QString, QString> res;
    QFile fl(fname);
    if (fl.open(QIODevice::ReadOnly))
    {
        QTextStream stream;
        stream.setDevice(&fl);
        stream.setCodec("UTF-8");
        QString curPath = QStringLiteral("/");
        while (1)
        {
            QString s = stream.readLine();
            if (s.isNull()) break;
            s = s.trimmed();
            if (s.isEmpty() || s[0] == QLatin1Char('#') || s[0] == QLatin1Char(';')) continue;
            if (s[0] == QLatin1Char('['))
            {
                // new path
                int len = s.length()-1;
                if (s[len] == QLatin1Char(']')) len--;
                s = s.mid(1, len).simplified();
                s += QLatin1Char('/');
                curPath = s;
                continue;
            }
            int eqp = s.indexOf(QLatin1Char('='));
            if (eqp < 0) continue; // invalid entry
            QString name = s.left(eqp).simplified();
            QString value = s.mid(eqp+1).simplified();
            if (name.isEmpty()) continue; // invalid entry
            name.prepend(curPath);
            if (forceLoCase) name = name.toLower();
            res.insert(name, value);
        }
        fl.close();
    }
    return res;
}

static inline void setXcursorInFile(const QString &fileName, const QString &themeName, int cursorSize)
{
    QStringList lst;
    {
        QFile fl(fileName);
        if (fl.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&fl);
            while (!stream.atEnd())
            {
                QString line = stream.readLine();
                if (!line.startsWith(QLatin1String("Xcursor.theme:"))
                    && !line.startsWith(QLatin1String("Xcursor.size:")))
                {
                    lst << line;
                }
            }
            fl.close();
        }
    }
    while (lst.size() > 0)
    {
        QString s(lst[lst.size()-1]);
        if (!s.trimmed().isEmpty()) break;
        lst.removeAt(lst.size()-1);
    }
    {
        QFile fl(fileName);
        if (fl.open(QIODevice::WriteOnly))
        {
            QTextStream stream(&fl);
            for (const QString &s : qAsConst(lst))
            {
                stream << s << "\n";
            }
            stream << "\nXcursor.theme: " << themeName << "\n";
            stream << "Xcursor.size: " << cursorSize << "\n";
            fl.close();
        }
    }
}

void setXcursor(const QString &themeName, int cursorSize)
{
    // NOTE: It should especially be set in "~/.Xresources", "~/.Xdefaults" being the old place.
    setXcursorInFile(QDir::home().path() + QStringLiteral("/.Xresources"), themeName, cursorSize);
    setXcursorInFile(QDir::home().path() + QStringLiteral("/.Xdefaults"), themeName, cursorSize);
}

const QString findDefaultTheme()
{
    QString res;
    QFile fl(QDir::home().path()+QStringLiteral("/.Xresources"));
    if (fl.open(QIODevice::ReadOnly))
    {
        QTextStream stream;
        stream.setDevice(&fl);
        while (1)
        {
            QString s = stream.readLine();
            if (s.isNull()) break;
            if (!s.startsWith(QLatin1String("Xcursor.theme:"))) continue;
            s.remove(0, 14);
            s = s.trimmed();
            if (s.isEmpty()) s = QLatin1String("default");
            res = s;
        }
        fl.close();
    }
    if (res.isEmpty())
    {
        QFile fl = QFile(QDir::home().path()+QStringLiteral("/.Xdefaults"));
        if (fl.open(QIODevice::ReadOnly))
        {
            QTextStream stream;
            stream.setDevice(&fl);
            while (1)
            {
                QString s = stream.readLine();
                if (s.isNull()) break;
                if (!s.startsWith(QLatin1String("Xcursor.theme:"))) continue;
                s.remove(0, 14);
                s = s.trimmed();
                if (s.isEmpty()) s = QLatin1String("default");
                res = s;
            }
            fl.close();
        }
    }
    if (res.isEmpty())
    {
        res = QStringLiteral("default");
    }
    return res;
}
