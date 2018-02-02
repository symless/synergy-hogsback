#ifndef SYNERGY_COMMON_SCREENSTATUS_H
#define SYNERGY_COMMON_SCREENSTATUS_H

#include <string>

enum class ScreenStatus: int {
    kConnected,
    kConnecting,
    kDisconnected,
    kInactive
};

inline ScreenStatus
stringToScreenStatus (std::string const& str)
{
    ScreenStatus status;

    if (str == "Connected") {
        status = ScreenStatus::kConnected;
    }
    else if ((str == "Connecting") || (str == "ConnectingWithError")) {
        status = ScreenStatus::kConnecting;
    }
    else if (str == "Disconnected") {
        status = ScreenStatus::kDisconnected;
    }
    else if (str == "Inactive") {
        status = ScreenStatus::kInactive;
    } else {
       throw;
    }

    return status;
}

inline std::string
screenStatusToString(ScreenStatus s)
{
    switch(s) {
    case ScreenStatus::kConnected:
        return "Connected";
    case ScreenStatus::kConnecting:
        return "Connecting";
    case ScreenStatus::kDisconnected:
        return "Disconnected";
    case ScreenStatus::kInactive:
        return "Inactive";
    }
}

#endif // SYNERGY_COMMON_SCREENSTATUS_H
