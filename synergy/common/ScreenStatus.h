#ifndef SYNERGY_COMMON_SCREENSTATUS_H
#define SYNERGY_COMMON_SCREENSTATUS_H

#include <string>

enum class ScreenStatus: int {
    kConnected,
    kConnecting,
    kDisconnected,
    kInactive,
    kConnectingWithError
};

inline ScreenStatus
stringToScreenStatus (std::string const& str)
{
    ScreenStatus status;

    if (str == "Connected") {
        status = ScreenStatus::kConnected;
    }
    else if (str == "Connecting") {
        status = ScreenStatus::kConnecting;
    }
    else if (str == "ConnectingWithError") {
        status = ScreenStatus::kConnectingWithError;
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
    case ScreenStatus::kConnectingWithError:
        return "ConnectingWithError";
    }
}

#endif // SYNERGY_COMMON_SCREENSTATUS_H
