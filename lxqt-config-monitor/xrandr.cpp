/*
    Copyright (C) 2014  P.L. Lucas <selairi@gmail.com>
    Copyright (C) 2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

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

#include "xrandr.h"
#include <QProcess>
#include <QDebug>
#include <QRegExp>
#include <QObject>
#include <QDir>
#include <QVector>

static QByteArray indentString(QByteArray line) {
  QByteArray s;
  int length = line.size();
  char ch;
  for(int indent=0; indent < length; indent++ ) {
    ch = line[indent];
    if( ch == ' ' || ch == '\t' )
      s.append(ch);
    else
      break;
  }
  return s;
}

static int indentLevel(QByteArray line) {
  int indent = 0;
  while(indent < line.size() && ( line[indent] == ' ' || line[indent] == '\t' ) )
    ++indent;
  return indent;
}

static int countLeadingWhiteSpaces(QByteArray line) {
  int indent = 0;
  while(indent < line.size() && ( line[indent] == ' ') )
    ++indent;
  return indent;
}

// new parsing code using xrandr --verbose
QList<MonitorInfo*> XRandRBackend::getMonitorsInfo() {
  QList<MonitorInfo*> monitors;
  // execute xrandr command and read its output
  QProcess process;
  // set locale to "C" guarantee English output of xrandr
  process.processEnvironment().insert("LC_ALL", "c");
  process.start("xrandr --verbose");
  //process.start("cat pruebas.txt");
  process.waitForFinished(-1);
  if(process.exitCode() != 0)
    return monitors;
  QList<QByteArray> lines = process.readAllStandardOutput().split('\n');
  // start parsing the output
  QRegExp regMonitorLine("([\\w-]+) +connected +(primary)? *(\\d+x\\d+\\+(\\d+)\\+(\\d+))?.*");
  QRegExp regModeLine("\\s+(\\d+x\\d+).*");
  QRegExp regRateLine("\\s+([vh]):.* clock\\s+([\\d.]+).?Hz.*");
  QRegExp regKeyValue("\\s*(\\w[\\w ]*)\\s*:\\s*(\\S.*)?");
  QRegExp regWidthLine("\\s+h:\\s+width\\s+([\\d.]+).*");
  QRegExp regHeightLine("\\s+v:\\s+height\\s+([\\d.]+).*");

  bool hasError = false;
  MonitorInfo* monitor = NULL;
  QList<QByteArray>::iterator it = lines.begin();
  bool readingModes = false;
  // currently, we only support one X screen, that is screen 0
  while(it != lines.end() && !hasError) {
    QByteArray& line = *it;
    if(!monitor) {
      if(regMonitorLine.exactMatch(line)) {
        // format: VGA-0 connected 1280x1024+1024+0 (0x55) normal...
        monitor = new MonitorInfo();
        monitor->name = regMonitorLine.cap(1);
        if(regMonitorLine.cap(2) == "primary") { // is primary monitor
          monitor->primaryOk = true;
        }
        if(!regMonitorLine.cap(3).isEmpty()) // mode+xpos+ypos
          monitor->enabledOk = true;
        monitor->xPos = regMonitorLine.cap(4).toInt();
        monitor->yPos = regMonitorLine.cap(5).toInt();
      }
    }
    else { // reading properties of this monitor
      if( regModeLine.exactMatch(line) || countLeadingWhiteSpaces(line)==2 ) { // this is a mode line
        // sample: 1280x1024 (0x55) 108.000MHz +HSync +VSync *current +preferred
        readingModes = true;
        // Mode name
        // QString mode = regModeLine.cap(1);
        QString mode = line.mid(0,line.lastIndexOf('(')).trimmed();
        QString rate;
        int width = -1;
        int height = -1;
        bool isCurrent = line.contains("current");
        bool isPreferred = line.contains("preferred");
        ++it;
        while(it != lines.end()) {
          line = *it;
          if(regWidthLine.exactMatch(line)) {
            width = regWidthLine.cap(1).toInt();
          }
          if(regHeightLine.exactMatch(line)) {
            height = regHeightLine.cap(1).toInt();
          }
          if(regRateLine.exactMatch(line)) {
            // sample:
            //        h: width  1280 start 1328 end 1440 total 1688 skew    0 clock  63.98KHz
            //        v: height 1024 start 1025 end 1028 total 1066           clock  60.02Hz
            if(regRateLine.cap(1) == QLatin1String("v"))
              rate = regRateLine.cap(2);
            ++it;
          }
          else {
            --it;
            break; // rate lines ended for this mode
          }
        }
        if(!mode.isEmpty() && !rate.isEmpty()) {
          if(!monitor->modes.contains(mode)) {
            monitor->modes.append(mode);
            monitor->monitorModes[mode] = new MonitorMode(mode, this);
          }
          monitor->monitorModes[mode]->modeLines.append(rate);
          monitor->monitorModes[mode]->width = width;
          monitor->monitorModes[mode]->height = height;
          if(isPreferred) {
            monitor->preferredMode = mode;
            monitor->preferredRate = rate;
          }
          if(isCurrent) {
            monitor->currentMode = mode;
            monitor->currentRate = rate;
          }
        }
      }
      else { // this is not a mode line, read other properties
        if(readingModes) {
          // mode lines ended, so the whole monitor info is read
          qDebug() << "Reading modes end";
          monitors.append(monitor);
          monitor = NULL;
          readingModes = false;
          continue;
        }

        if(regKeyValue.exactMatch(line)) { // format: <key>: <value>
          QString key = regKeyValue.cap(1);
          QString value = regKeyValue.cap(2);
          QByteArray lineStringStart = indentString(line);
          int propertyIndentLevel = indentLevel(line);
          ++it;
          while( it != lines.end() ) {
            QByteArray& line = *it;
            int actualIndentLevel = indentLevel(line);
            if( actualIndentLevel>propertyIndentLevel && line.startsWith(lineStringStart) ) {
              value += "\n" + line.trimmed();
              ++it;
            } else
              break;
          }
          qDebug() << key << "=" << value;
          if(key == "Gamma") {
            monitor->gamma = value;
          }
          else if(key == "Brightness") {
            monitor->brightness = value;
          }
          else if(key == "EDID") {
            monitor->edid = value ;
            // Get vendor
            QString hex = value.replace("\n","").replace(" ","").toLower();
            int vendorPosStart = hex.indexOf("fc00");
            if(vendorPosStart>0) {
            int vendorPosEnd = hex.indexOf("00", vendorPosStart+4);
              QString vendorHex = hex.mid(vendorPosStart+4, vendorPosEnd-vendorPosStart-4);
              QByteArray vendor;
              //vendor  = QByteArray::fromHex(vendorHex.toLocal8Bit()).trimmed();
              //qDebug() << "VendorHex:" << vendorHex << "Vendor" << vendor ;
              // QByteArray::fromHex sometimes fails. This a trick
              vendor="";
              for(int i=1; i<vendorHex.length();i+=2) {
                vendor+=QByteArray::fromHex(QString("%1%2").arg(vendorHex.at(i-1)).arg(vendorHex.at(i)).toLocal8Bit());
              }
              qDebug() << "VendorHex:" << vendorHex << "Vendor" << vendor ;
              monitor->vendor = vendor;
            }
          }
          else if(key == "Backlight") {
            QRegExp rx("(\\d+)");
            QStringList list;
            int pos = 0;
            while ((pos = rx.indexIn(value, pos)) != -1) {
              list << rx.cap(1);
              pos += rx.matchedLength();
            }
            if(list.length()==3) {
              monitor->backlight=list[0];
              monitor->backlightMin=list[1];
              monitor->backlightMax=list[2];
            }
          }
          continue;
        } // End format: <key>: <value>
        else { // this line is not key:value
        }
      }
    }
    if( it != lines.end() ) ++it;
  }

  if(monitor) // this should not happen unless a parsing error happens
    delete monitor;

  return monitors;
}



bool XRandRBackend::setMonitorsSettings(const QList<MonitorSettings*> monitors) {
  QString cmd = getCommand(monitors);
  qDebug() << cmd;
  // return true;
  QProcess process;
  process.start(cmd);
  process.waitForFinished();
  return process.exitCode() == 0;
}




QString XRandRBackend::getCommand(const QList<MonitorSettings*> monitors)  {

  QByteArray cmd = "xrandr";

  foreach(MonitorSettings * monitor, monitors) {
    cmd += " --output ";
    cmd.append(monitor->name);
    cmd.append(' ');

    // if the monitor is turned on
    if(monitor->enabledOk) {
      QString sel_res = monitor->currentMode;
      QString sel_rate = monitor->currentRate;

      if(sel_res == QObject::tr("Auto"))   // auto resolution
        cmd.append("--auto");
      else {
        cmd.append("--mode \"");
        cmd.append(sel_res);
        cmd.append('"');
        if(sel_rate != QObject::tr("Auto")) {  // not auto refresh rate
          cmd.append(" --rate ");
          cmd.append(sel_rate);
        }
      }
      if(monitor->position == MonitorSettings::Manual) { // Manual position
        cmd.append(QString(" --pos %1x%2").arg(monitor->xPos).arg(monitor->yPos));
      } else // Unify output
        cmd.append(QString(" --pos 0x0"));
      if(monitor->primaryOk)
        cmd.append(" --primary");
      cmd.append(" --brightness ");
      cmd.append(monitor->brightness);
      cmd.append(" --gamma ");
      cmd.append(monitor->gamma);
      if( !monitor->backlight.isEmpty() ) {
        cmd.append(" --set Backlight ");
        cmd.append(monitor->backlight);
      }
    }
    else    // turn off
      cmd.append("--off");
  }


  qDebug() << "cmd:" << cmd;
  return cmd;
}

