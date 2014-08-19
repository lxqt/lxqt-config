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

#ifndef _XRANDR_H_
#define _XRANDR_H_

#include "monitor.h"
#include <QList>


class XRandRBackend: public Backend {
	Q_OBJECT
public:
  // Execute xrandr command and read its output
  QList<MonitorInfo*> getMonitorsInfo();
  // Set changes in xrandr
  bool setMonitorsSettings(const QList<MonitorSettings*> monitors);
  QString getCommand(const QList<MonitorSettings*> monitors);
};

#endif