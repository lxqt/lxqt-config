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


#ifndef _MONITORPICTURE_H_
#define _MONITORPICTURE_H_

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include "monitor.h"

class MonitorPicture: public QGraphicsRectItem {

public:
  MonitorPicture(QGraphicsItem * parent, MonitorInfo *monitorInfo);
  void setMonitorPosition(int x, int y);
  void adjustNameSize();
    
private:
  QGraphicsTextItem *textItem;
  MonitorInfo *monitorInfo;

protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant & value);
};


/**MonitorPicture is not a QObject child. MonitorPictureQOboject is used to provide SLOTs to MonitorPicture.*/
class MonitorPictureQObject: public QObject {
Q_OBJECT
public:
  MonitorPictureQObject(MonitorPicture *monitorPicture, QObject *parent=0);

public Q_SLOTS:
  void setXMonitorPosition(int x);
  void setYMonitorPosition(int y);
  void setSize(QSize size);
  void setSize(const QString rate);

private:
  MonitorPicture *monitorPicture;
};

#endif // _MONITORPICTURE_H_
