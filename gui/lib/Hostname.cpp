#include "Hostname.h"

#include <QHostInfo>

Hostname::Hostname()
{
	m_hostname = QHostInfo::localHostName();
}

QString Hostname::hostname()
{
	return m_hostname;
}
