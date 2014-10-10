/*
 * translationloader.cpp - an Qt5 automatic translations loader for libs
 * Copyright (C) 2014  Luís Pereira <luis.artur.pereira@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef TRANSLATION_LOADER_H
#define TRANSLATION_LOADER_H

#include <QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
#include <LXQt/Translator>
#include <QCoreApplication>

static void loadTranslation()
{
    LxQt::Translator::translateLibrary(QLatin1String(PROJECT_NAME));
}

Q_COREAPP_STARTUP_FUNCTION(loadTranslation)
#endif

#endif // TRANSLATION_LOADER_H
