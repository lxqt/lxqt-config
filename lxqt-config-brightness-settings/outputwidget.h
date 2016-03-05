/*
    Copyright (C) 2015  P.L. Lucas <selairi@gmail.com>
    
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

#ifndef __OUTPUT_WIDGET_H__
#define __OUTPUT_WIDGET_H__

#include <QGroupBox>
#include "monitorinfo.h"
#include "ui_outputwidget.h"

class OutputWidget: public QWidget
{
Q_OBJECT
public:
    OutputWidget(MonitorInfo monitor, QWidget *parent);

signals:
    void changed(MonitorInfo info);

public slots:
    void backlightChanged(int value);
    void brightnessChanged(int value);
private:
    MonitorInfo mMonitor;
    Ui::OutputWidget *ui;
};

#endif

