#ifndef PROCESSCOMMAND_H
#define PROCESSCOMMAND_H

#include "LibMacro.h"
#include "DebugLevel.h"

#include <QObject>

class DirectoryManager;

class LIB_SPEC ProcessCommand : public QObject
{
	Q_OBJECT

public:

	explicit ProcessCommand(QObject* parent = 0);

	QString command(bool serverMode) const;
	QStringList arguments(bool serverMode) const;
	void setServerIp(const QString ip);
	void setDirectoryManager(DirectoryManager* dm);

private:
	QString wrapCommand(QString command) const;

private:
    QString m_serverIp;
	DirectoryManager* m_directoryManager;
};

#endif // PROCESSCOMMAND_H
