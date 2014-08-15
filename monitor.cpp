#include "monitor.h"

Monitor::Monitor(QObject *parent) : QObject(parent) {
  enable = NULL;
  resolutionCombo = rateCombo = NULL;
}

MonitorSettings::MonitorSettings(QObject *parent): QObject(parent) {
  position = None;
}

MonitorInfo::MonitorInfo(QObject *parent): MonitorSettings(parent) {
}