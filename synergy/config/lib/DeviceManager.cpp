#include "DeviceManager.h"

#include <QApplication>
#include <QDesktopWidget>

DeviceManager::DeviceManager()
{

}

DeviceManager::~DeviceManager()
{

}

int DeviceManager::resolutionWidth()
{
	return QApplication::desktop()->width();
}

int DeviceManager::resolutionHeight()
{
	return QApplication::desktop()->height();
}

int DeviceManager::primaryMonitorWidth()
{
	QRect rec = QApplication::desktop()->screenGeometry();
	return  rec.width();
}

int DeviceManager::primaryMonitorHeight()
{
	QRect rec = QApplication::desktop()->screenGeometry();
	return rec.height();
}
