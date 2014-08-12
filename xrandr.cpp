#include "xrandr.h"
#include <QProcess>
#include <QDebug>
#include <QRegExp>

QList<Monitor*> readXRandRInfo()
{
  // execute xrandr command and read its output
  QList<Monitor*> monitors;
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
      Monitor *monitor = new Monitor();
      monitor->currentMode = monitor->currentRate = -1;
      monitor->preferredMode = monitor->preferredRate = -1;
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
            monitor->currentMode = imode;
            monitor->currentRate = irate;
          }
          if(rate.contains("+")) { // preferred mode
            monitor->preferredMode = imode;
            monitor->preferredRate = irate;
          }
        }
      }
    }  
  }  
  return monitors;  
}