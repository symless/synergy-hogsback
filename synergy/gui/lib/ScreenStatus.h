#ifndef SCREENSTATUS
#define SCREENSTATUS

#include <QString>

enum ScreenStatus {
    kConnected,
    kConnecting,
    kDisconnected,
    kInactive
};

static ScreenStatus
stringToScreenStatus(QString str)
{
    ScreenStatus status = kInactive;

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
screenStatusToString(ScreenStatus s)
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


#endif // SCREENSTATUS
