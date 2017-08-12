#ifndef SCREENSTATUS
#define SCREENSTATUS

#include <string>

enum class ScreenStatus: int {
    kConnected,
    kConnecting,
    kDisconnected,
    kInactive,
    kConnectingWithError
};

static ScreenStatus
stringToScreenStatus(std::string str)
{
    ScreenStatus status = ScreenStatus::kInactive;

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
    }

    return status;
}

static std::string
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

    return "Inactive";
}


#endif // SCREENSTATUS
