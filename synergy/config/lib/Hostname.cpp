#include "Hostname.h"

#include <synergy/common/Hostname.h>

#include <QHostInfo>

Hostname::Hostname()
{
    m_hostname = QString::fromStdString(localHostname());
}

QString Hostname::hostname()
{
	return m_hostname;
}
