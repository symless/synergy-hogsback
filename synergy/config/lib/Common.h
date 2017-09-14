#ifndef COMMON_H
#define COMMON_H

#include "LibMacro.h"
#include <QString>

#ifndef NULL
#define NULL 0
#endif

LIB_SPEC extern const int kScreenIconWidth;
LIB_SPEC extern const int kScreenIconHeight;

LIB_SPEC extern const int kDefaultViewWidth;
LIB_SPEC extern const int kDefaultViewHeight;

LIB_SPEC extern const int kSnappingThreshold;
LIB_SPEC extern const float kScaleThreshold;

LIB_SPEC extern const QString kDefaultConfigFile;

#endif // COMMON_H
