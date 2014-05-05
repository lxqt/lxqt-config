/*
 * Copyright (C) 2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 * LXQt project: http://lxde.org/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "fontconfigfile.h"
#include <QTextStream>
#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QStringBuilder>
#include <QDomDocument>
#include <QTimer>
#include <QDebug>

FontConfigFile::FontConfigFile(QObject* parent):
    QObject(parent),
    mAntialias(true),
    mHinting(true),
    mSubpixel("rgb"),
    mHintStyle("hintslight"),
    mDpi(96),
    mSaveTimer(NULL)
{
    mDirPath = QString::fromLocal8Bit(qgetenv("XDG_CONFIG_HOME"));
    if(mDirPath.isEmpty())
        mDirPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation) % "/.config";
    mDirPath += "/fontconfig";
    mFilePath = mDirPath += "/fonts.conf";

    load();
}

FontConfigFile::~FontConfigFile()
{
    if(mSaveTimer) // has pending save request
    {
        delete mSaveTimer;
	mSaveTimer = NULL;
        save();
    }
}

void FontConfigFile::setAntialias(bool value)
{
    mAntialias = value;
    queueSave();
}

void FontConfigFile::setSubpixel(QByteArray value)
{
    mSubpixel = value;
    queueSave();
}

void FontConfigFile::setHinting(bool value)
{
    mHinting = value;
    queueSave();
}

void FontConfigFile::setHintStyle(QByteArray value)
{
    mHintStyle = value;
    queueSave();
}

void FontConfigFile::setDpi(int value)
{
    mDpi = value;
    queueSave();
}

void FontConfigFile::load()
{
    QFile file(mFilePath);
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray buffer = file.readAll();
        file.close();
        if(buffer.contains("lxqt-config-appearance")) // the config file is created by us
        {
            // doing full xml parsing is over-kill. let's use some simpler brute-force methods.
            QDomDocument doc;
            doc.setContent(&file);
            file.close();
            QDomElement docElem = doc.documentElement();
            QDomNodeList editNodes = docElem.elementsByTagName("edit");
            for(int i = 0; i < editNodes.count(); ++i)
            {
                QDomElement editElem = editNodes.at(i).toElement();
                QByteArray name = editElem.attribute("name").toLatin1();
                if(name == "antialias")
                {
                    QString value = editElem.firstChildElement("bool").text();
                    mAntialias = value[0] == 't' ? true : false;
                }
                else if(name == "rgba")
                {
                    QString value = editElem.firstChildElement("const").text();
                    mSubpixel = value.toLatin1();
                }
                else if(name == "hinting")
                {
                    QString value = editElem.firstChildElement("bool").text();
                    mHinting = value[0] == 't' ? true : false;
                }
                else if(name == "hintstyle")
                {
                    QString value = editElem.firstChildElement("const").text();
                    mHintStyle = value.toLatin1();
                }
                else if(name == "dpi")
                {
                    QString value = editElem.firstChildElement("double").text();
                    mDpi = value.toInt();
                }
            }
        }
        else // the config file is created by others => make a backup and write our config
        {
            QFile backup(mFilePath + ".bak");
            if(backup.open(QIODevice::WriteOnly))
            {
                backup.write(buffer);
                backup.close();
            }
            queueSave(); // overwrite with our file
        }
    }
}

void FontConfigFile::save()
{
    if(mSaveTimer)
    {
        mSaveTimer->deleteLater();
        mSaveTimer = NULL;
    }

    QFile file(mFilePath);
    QDir().mkdir(mDirPath);
    // References: https://wiki.archlinux.org/index.php/Font_Configuration
    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream s(&file);
        s <<
        "<?xml version=\"1.0\"?>\n"
        "<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">\n"
        "<!-- created by lxqt-config-appearance (DO NOT EDIT!) -->\n"
        "<fontconfig>\n"
        "  <match target=\"font\">\n"
        "    <edit name=\"antialias\" mode=\"assign\">\n"
        "      <bool>" << (mAntialias ? "true" : "false") << "</bool>\n"
        "    </edit>\n"
        "    <edit name=\"rgba\" mode=\"assign\">\n"
        "      <const>" << mSubpixel << "</const>\n"
        "    </edit>\n"
        "    <edit name=\"lcdfilter\" mode=\"assign\">\n"
        "      <const>lcddefault</const>\n"
        "    </edit>\n"
        "    <edit name=\"hinting\" mode=\"assign\">\n"
        "      <bool>" << (mHinting ? "true" : "false") << "</bool>\n"
        "    </edit>\n"
        "    <edit name=\"hintstyle\" mode=\"assign\">\n"
        "      <const>" << mHintStyle << "</const>\n"
        "    </edit>\n"
        "  </match>\n"
        "  <match target=\"pattern\">\n"
        "    <edit name=\"dpi\" mode=\"assign\">\n"
        "      <double>" << mDpi << "</double>\n"
        "    </edit>\n"
        "  </match>\n"
        "</fontconfig>";
        s.flush();
        file.close();
    }
}

void FontConfigFile::queueSave()
{
    if(mSaveTimer)
        mSaveTimer->start(1500);
    else
    {
        mSaveTimer = new QTimer();
        mSaveTimer->setSingleShot(true);
        connect(mSaveTimer, SIGNAL(timeout()), this, SLOT(save()));
        mSaveTimer->start(1500);
    }
}

