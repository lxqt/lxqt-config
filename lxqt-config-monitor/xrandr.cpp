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

int indentLevel(QByteArray line) {
  int indent = 0;
  while(indent < line.size() && line[indent] == ' ' || line[indent] == '\t')
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
  process.waitForFinished(-1);
  if(process.exitCode() != 0)
    return monitors;
  QList<QByteArray> lines = process.readAllStandardOutput().split('\n');
  // start parsing the output
  QRegExp regMonitorLine("([\\w-]+) +connected +(primary)? *(\\d+x\\d+\\+(\\d+)\\+(\\d+))?.*");
  QRegExp regModeLine("\\s+(\\d+x\\d+).*");
  QRegExp regRateLine("\\s+([vh]):.* clock\\s+([\\d.]+).?Hz.*");
  QRegExp regKeyValue("\\s*(\\w[\\w ]*)\\s*:\\s*(\\S.*)?");

  bool hasError = false;
  MonitorInfo* monitor = NULL;
  QByteArray edid;
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
      if(regModeLine.exactMatch(line)) { // this is a mode line
        // sample: 1280x1024 (0x55) 108.000MHz +HSync +VSync *current +preferred
        readingModes = true;
        QString mode = regModeLine.cap(1);
        QString rate;
        bool isCurrent = line.contains("current");
        bool isPreferred = line.contains("preferred");
        ++it;
        while(it != lines.end()) {
          line = *it;
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
          if(!monitor->modes.contains(mode))
            monitor->modes.append(mode);
          QStringList& modeLine = monitor->modeLines[mode];
          modeLine.append(rate);
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
          monitors.append(monitor);
          monitor = NULL;
          readingModes = false;
          continue;
        }

        if(regKeyValue.exactMatch(line)) { // format: <key>: <value>
          QString key = regKeyValue.cap(1);
          QString value = regKeyValue.cap(2);
          // qDebug() << key << "=" << value;
          if(key == "Gamma") {
            monitor->gamma = value;
            ++it;
          }
          else if(key == "Brightness") {
            monitor->brightness = value;
            ++it;
          }
          else if(key == "EDID") {
            int indent = indentLevel(line);
            ++it;
            // start reading EDID data
            while(it != lines.end()) {
              line = *it;
              int edidIndent = indentLevel(line);
              if(edidIndent > indent) {
                edid += line.trimmed();
                ++it;
              }
              else { // end of EDID
                monitor->edid = QByteArray::fromHex(edid);
                qDebug() << "EDID:" << edid;
                edid.clear();
                --it;
                break;
              }
            }
          }
        }
        else { // this line is not key:value
        }
      }
    }
    ++it;
  }

  if(monitor) // this should not happen unless a parsing error happens
    delete monitor;

  resolvePositions(monitors);

  return monitors;
}

#if 0 // old lxqt-config-randr code using xrandr --query

QList<MonitorInfo*> XRandRBackend::getMonitorsInfo() {
  // execute xrandr command and read its output
  QList<MonitorInfo*> monitors;
  QProcess process;
  // set locale to "C" guarantee English output of xrandr
  process.processEnvironment().insert("LC_ALL", "c");
  process.start("xrandr");
  // process.start("cat testing.txt");
  process.waitForFinished(-1);
  if(process.exitCode() != 0)
    return monitors;

  QByteArray output = process.readAllStandardOutput();

  QRegExp regex("((?:[a-zA-Z]+[-0-9]*) +connected [^\n]+\n(?:(?: +[0-9]+x[0-9]+[^\n]+\n)+))");
  QRegExp re_name("([a-zA-Z]+[-0-9]*) +connected .*\n");
  QRegExp re_rate("[0-9\\.]+");


  //Get output of xrandr for each monitor
  int index = 0;
  while((index = regex.indexIn(output.constData(), index)) >= 0) {
    index += regex.matchedLength();
    if(regex.matchedLength() > 0) {
      QString input = regex.cap(0);
      // New monitor found
      qDebug() << input;
      MonitorInfo* monitor = new MonitorInfo();
      monitor->currentMode = monitor->currentRate = "";
      monitor->preferredMode = monitor->preferredRate = "";
      monitor->enabledOk = false;
      int imode = -1;
      monitors.append(monitor);
      // Get monitor name
      if(re_name.indexIn(input) >= 0)
        monitor->name = re_name.cap(1);
      qDebug() << "Name:" << monitor->name;
      // Get monitor modes
      QStringList lines = input.split("\n");
      for(int i = 1; i < lines.length(); i++) {
        qDebug() << "Mode:" << lines[i];
        if(lines[i].trimmed() == "")
          continue;
        QStringList rates = lines[i].trimmed().split(" ");
        QString mode = rates[0]; // 1st rate is mode name
        qDebug() << "Mode found:" << mode;
        QStringList aux_rates;
        monitor->modeLines[mode] = aux_rates;
        monitor->modes.append(mode);
        imode++;
        int irate = -1;
        // Get rates for this mode
        for(int j = 1; j < rates.length(); j++) {
          QString rate = rates[j];
          if(rate.contains(re_rate)) {
            irate++;
            QString _rate = rate;
            _rate.replace("*", "").replace("+", "");
            monitor->modeLines[mode].append(_rate);
            qDebug() << "Rate:" << rate;
          }
          if(rate.contains("*")) { // current mode
            monitor->currentMode = monitor->modes[imode];
            monitor->currentRate = monitor->modeLines[mode][irate];
            monitor->currentMode = regMonitorLine.cap(3);
            monitor->enabledOk = true;
          }
          if(rate.contains("+")) { // preferred mode
            monitor->preferredMode = monitor->modes[imode];
            monitor->preferredRate = monitor->modeLines[mode][irate];
          }
        }
      }
    }
  }
  return monitors;
}

#endif


bool XRandRBackend::setMonitorsSettings(const QList<MonitorSettings*> monitors) {
  QString cmd = getCommand(monitors);
  QProcess process;
  process.start(cmd);
  process.waitForFinished();
  return process.exitCode() == 0;
}




QString XRandRBackend::getCommand(const QList<MonitorSettings*> monitors)  {
  QMap<MonitorSettings::Position, QString> positions;

  positions[MonitorSettings::Left] = "--left-of";
  positions[MonitorSettings::Right] = "--right-of";
  positions[MonitorSettings::Above] = "--above";
  positions[MonitorSettings::Bellow] = "--bellow";


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
        cmd.append("--mode ");
        cmd.append(sel_res);

        if(sel_rate != QObject::tr("Auto")) {  // not auto refresh rate
          cmd.append(" --rate ");
          cmd.append(sel_rate);
        }
        if(monitor->position != MonitorSettings::None) {
          cmd.append(" ");
          cmd.append(positions[monitor->position]);
          cmd.append(" ");
          cmd.append(monitor->positionRelativeToOutput);
        }
        if(monitor->primaryOk)
          cmd.append(" --primary");
      }
    }
    else    // turn off
      cmd.append("--off");
  }


  qDebug() << "cmd:" << cmd;
  return cmd;
}

// resolve the position relationship among the montors
void XRandRBackend::resolvePositions(QList< MonitorInfo* >& monitors) {
  Q_FOREACH(MonitorInfo * monitor, monitors) {
    MonitorSettings::Position pos;
    MonitorInfo* neighbor = findAdjacentMonitor(monitors, monitor, pos);
    if(neighbor) {
      monitor->position = pos;
      monitor->positionRelativeToOutput = neighbor->name;
    }
  }
}

MonitorInfo* XRandRBackend::findAdjacentMonitor(QList< MonitorInfo* >& monitors, MonitorInfo* monitor, MonitorSettings::Position& pos) {
  MonitorInfo* neighbor = NULL;
  QRect monitorRect = monitor->geometry();
  pos = MonitorSettings::None;
  Q_FOREACH(MonitorInfo * mon, monitors) {
    if(mon == monitor)
      continue;
    QRect neighborRect = mon->geometry();
    if(monitorRect.top() == neighborRect.top()) {
      if((monitorRect.right() + 1) == neighborRect.left()) { // monitor is at left of neighbor
        pos = MonitorSettings::Left;
        neighbor = mon;
        break;
      }
      else if(monitorRect.left() == (neighborRect.right() + 1)) { // monitor is at right of neighbor
        pos = MonitorSettings::Right;
        neighbor = mon;
        break;
      }
    }
    if(monitorRect.left() == neighborRect.left()) {
      if((monitorRect.bottom() + 1) == neighborRect.top()) { // monitor is above neighbor
        pos = MonitorSettings::Above;
        neighbor = mon;
        break;
      }
      else if(monitorRect.top() == (neighborRect.bottom() + 1)) { // monitor is below neighbor
        pos = MonitorSettings::Bellow;
        neighbor = mon;
        break;
      }
    }
  }
  return neighbor;
}
