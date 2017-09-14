#ifndef DEBUGLEVELUTIL_H
#define DEBUGLEVELUTIL_H

#include "synergy/common/DebugLevel.h"

#include <QString>

// TODO: use Q_ENUMS to convert enum to string
QString debugLevelToStr(DebugLevel value);

#endif // DEBUGLEVELUTIL_H
