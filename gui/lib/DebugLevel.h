#ifndef DEBUGLEVEL_H
#define DEBUGLEVEL_H

#include <QString>

enum DebugLevel {
	kError,
	kWarning,
	kNote,
	kInfo,
	kDebug,
	kDebug1,
	kDebug2
};

// TODO: use Q_ENUMS to convert enum to string
QString debugLevelToStr(DebugLevel value);

#endif // DEBUGLEVEL_H
