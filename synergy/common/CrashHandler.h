#ifndef SYNERGY_COMMON_CRASHHANDLER_H
#define SYNERGY_COMMON_CRASHHANDLER_H

#if defined(SYNERGY_CRASHPAD)
bool startCrashHandler();
#else
inline bool startCrashHandler() { return false; }
#endif

#endif // SYNERGY_COMMON_CRASHHANDLER_H
