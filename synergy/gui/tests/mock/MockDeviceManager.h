#ifndef MOCKDEVICEMANAGER_H
#define MOCKDEVICEMANAGER_H

#include "DeviceManager.h"

class MockDeviceManager : public DeviceManager
{
public:
    MockDeviceManager();

    int resolutionWidth();
    int resolutionHeight();
    int primaryMonitorWidth();
    int primaryMonitorHeight();
};

#endif // MOCKDEVICEMANAGER_H
