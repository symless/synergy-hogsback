#ifndef SCREENSTATE
#define SCREENSTATE

#include <QString>

enum ScreenState {
    kConnected,
    kConnecting,
    kDisconnected,
    kInactive
};

static ScreenState
stringToScreenState(QString str)
{
    ScreenState status = kInactive;

    if (str.contains("Connected", Qt::CaseInsensitive)) {
        status = kConnected;
    }
    else if (str.contains("Connecting", Qt::CaseInsensitive)) {
        status = kConnecting;
    }
    else if (str.contains("Disconnected", Qt::CaseInsensitive)) {
        status = kDisconnected;
    }
    else if (str.contains("Inactive", Qt::CaseInsensitive)) {
        status = kInactive;
    }

    return status;
}

static QString
screenStateToString(ScreenState s)
{
    switch(s) {
    case kConnected:
        return "Connected";
    case kConnecting:
        return "Connecting";
    case kDisconnected:
        return "Disconnected";
    case kInactive:
        return "Inactive";
    }

    return "Inactive";
}


#endif // SCREENSTATE
