#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <QComboBox>
#include <QCheckBox>
#include <QStringList>
#include <QHash>
#include <QList>

// Monitor info
class Monitor : public QObject {
Q_OBJECT

public:
  Monitor(QObject *parent = 0);
  QString name;
  QStringList modes; // Suported modes in order

  QCheckBox* enable;
  QComboBox* resolutionCombo;
  QComboBox* rateCombo;
};


//Settings to pass to backend
class MonitorSettings: public QObject {
	Q_OBJECT
public:
  MonitorSettings(QObject *parent = 0);
  QString name;
  QString currentMode;
  QString currentRate;
  bool enabledOk; 
  enum Position {None, Left, Right, Above, Bellow};  
  Position position;
  QString positionRelativeToOutput;
};


// Monitor information from backend
class MonitorInfo: public MonitorSettings {
	Q_OBJECT
public:
  MonitorInfo(QObject *parent = 0);
  QStringList modes; // Modes of this monitor in order
  QHash<QString, QStringList> modeLines; // Rates suported by each mode
  QString preferredMode;
  QString preferredRate;
};


class Backend: public QObject {
	Q_OBJECT
public:
  virtual QList<MonitorInfo*> getMonitorsInfo() = 0;
  virtual bool setMonitorsSettings(const QList<MonitorSettings*> monitors) = 0;
};

#endif // _MONITOR_H_