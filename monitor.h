#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <QComboBox>
#include <QCheckBox>
#include <QStringList>
#include <QHash>

// Monitor info
class Monitor : public QObject {
Q_OBJECT

public:
  QString name;
  QStringList modes; // Modes of this monitor
  QHash<QString, QStringList> modeLines; // Rates suported by each mode
  short currentMode;
  short currentRate;
  short preferredMode;
  short preferredRate;

  QCheckBox* enable;
  QComboBox* resolutionCombo;
  QComboBox* rateCombo;
};

// Q_DECLARE_METATYPE(Monitor*);
// Q_DECLARE_OPAQUE_POINTER( Monitor*);

#endif // _MONITOR_H_