#include "DebugLevelUtil.h"

QString debugLevelToStr(DebugLevel value)
{
	switch(value) {
	case kError:
		return "Error";
	case kWarning:
		return "Warning";
	case kNote:
		return "Note";
	case kInfo:
		return "Info";
	case kDebug:
		return "Debug";
	case kDebug1:
		return "Debug1";
	case kDebug2:
		return "Debug2";
	}

	return "Unknown";
}
