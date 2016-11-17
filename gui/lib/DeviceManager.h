#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "LibMacro.h"

#include <QObject>

class LIB_SPEC DeviceManager : QObject
{
	Q_OBJECT

public:
	DeviceManager();
	virtual ~DeviceManager();

	virtual int resolutionWidth();
	virtual int resolutionHeight();
	virtual int primaryMonitorWidth();
	virtual int primaryMonitorHeight();
};

#endif // DEVICEMANAGER_H
