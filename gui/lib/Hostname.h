#ifndef HOSTNAME_H
#define HOSTNAME_H

#include "LibMacro.h"

#include <QQuickItem>

class LIB_SPEC Hostname : public QQuickItem
{
	Q_OBJECT
public:
	Hostname();

	Q_INVOKABLE QString hostname();

private:
	QString m_hostname;
};

#endif // HOSTNAME_H
