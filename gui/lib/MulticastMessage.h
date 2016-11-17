#ifndef MULTICASTMESSAGE_H
#define MULTICASTMESSAGE_H

#include "LibMacro.h"
#include "MulticastMessageType.h"

#include <QString>

class LIB_SPEC MulticastMessage
{
public:
	MulticastMessage();
	MulticastMessage(const QString& message);

	QByteArray toByteArray();

	bool m_valid;
	int m_type;
	int m_processMode;
	bool m_active;
	bool m_joinGroup;
	QString m_hostname;
	QString m_ip;
	QString m_uniqueGroup;
	QString m_configInfo;

private:
	void parse(const QString& message);
};

#endif // MULTICASTMESSAGE_H
