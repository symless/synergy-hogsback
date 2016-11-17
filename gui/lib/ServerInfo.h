#ifndef SERVERINFO_H
#define SERVERINFO_H

#include "LibMacro.h"

#include<QString>

class LIB_SPEC ServerInfo
{
public:
	ServerInfo(QString name, QString ip, QString group);

	QString m_name;
	QString m_ip;
	QString m_uniqueGroup;
};

#endif // SERVERINFO_H
