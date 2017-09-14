#ifndef IPMANAGER_H
#define IPMANAGER_H

#include <QString>

class IpManager
{
public:
	IpManager();

	QString getLocalIPAddresses();
	QStringList getAllLocalIPAddresses();
};

#endif // IPMANAGER_H
