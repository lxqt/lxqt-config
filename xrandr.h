#ifndef _XRANDR_H_
#define _XRANDR_H_

#include "monitor.h"
#include <QList>

// Execute xrandr command and read its output
QList<Monitor*> readXRandRInfo();

#endif