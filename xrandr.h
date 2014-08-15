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
};

#endif