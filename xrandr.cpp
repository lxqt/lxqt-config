#include "xrandr.h"
#include <QProcess>
#include <QDebug>
#include <QRegExp>
#include <QObject>

QList<MonitorInfo*> XRandRBackend::getMonitorsInfo()
{
  // execute xrandr command and read its output
  QList<MonitorInfo*> monitors;
  QProcess process;
  // set locale to "C" guarantee English output of xrandr
  process.processEnvironment().insert("LC_ALL", "c");
  process.start("xrandr");
  process.waitForFinished(-1);
  if(process.exitCode() != 0)
    return monitors;

  QByteArray output = process.readAllStandardOutput();
  
  QRegExp regex("((?:[a-zA-Z]+[-0-9]*) +connected [^\n]+\n(?:(?: +[0-9]+x[0-9]+[^\n]+\n)+))");
  QRegExp re_name("([a-zA-Z]+[-0-9]*) +connected .*\n");
  QRegExp re_rate("[0-9\\.]+");
  

  //Get output of xrandr for each monitor
  int index = 0;
  while( index = regex.indexIn(output.constData(), index)>=0 ) {
    index += regex.matchedLength();
    if(regex.matchedLength()>0) {
      QString input = regex.cap(0);
      // New monitor found
      qDebug() << input;
      MonitorInfo *monitor = new MonitorInfo();
      monitor->currentMode = monitor->currentRate = "";
      monitor->preferredMode = monitor->preferredRate = "";
      monitor->enabledOk = false;
      int imode = -1;
      monitors.append(monitor);
      // Get monitor name
      if(re_name.indexIn(input)>=0)
        monitor->name=re_name.cap(1);
      qDebug() << "Name:" << monitor->name;
      // Get monitor modes
      QStringList lines = input.split("\n");
      for(int i=1;i<lines.length();i++) {
        qDebug() << "Mode:" << lines[i];
        if( lines[i].trimmed() == "" )
          continue;
        QStringList rates = lines[i].trimmed().split(" ");
        QString mode = rates[0]; // 1st rate is mode name
        qDebug() << "Mode found:" << mode;
        QStringList aux_rates;
        monitor->modeLines[mode] = aux_rates;
        monitor->modes.append(mode);
        imode++;
        int irate=-1;
        // Get rates for this mode
        for(int j=1; j<rates.length(); j++) {
          QString rate = rates[j];
          if(rate.contains(re_rate)) {
            irate++;
            QString _rate=rate;
            _rate.replace("*","").replace("+","");
            monitor->modeLines[mode].append(_rate);
            qDebug() << "Rate:" << rate;
          }
          if(rate.contains("*")) { // current mode
            monitor->currentMode = monitor->modes[imode];
            monitor->currentRate = monitor->modeLines[mode][irate];
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


bool XRandRBackend::setMonitorsSettings(const QList<MonitorSettings*> monitors) {
  
  QByteArray cmd = "xrandr";

  foreach(MonitorSettings *monitor, monitors) {
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

        if(sel_rate != QObject::tr("Auto") ) { // not auto refresh rate
          cmd.append(" --rate ");
          cmd.append(sel_rate);
        }
      }
    }
    else    // turn off
      cmd.append("--off");
  }
  
  
  qDebug() << "cmd:" << cmd;
  ;
  QProcess process;
  process.start(cmd);
  process.waitForFinished();
  return process.exitCode() == 0;
}
