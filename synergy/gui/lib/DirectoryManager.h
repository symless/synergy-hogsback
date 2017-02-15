#ifndef DIRECTORYMANAGER_H
#define DIRECTORYMANAGER_H

#include "LibMacro.h"

#include <QObject>

class LIB_SPEC DirectoryManager : QObject
{
	Q_OBJECT

public:
	DirectoryManager();
	virtual ~DirectoryManager();

	virtual QString profileDir();
	virtual QString configFileDir();
	virtual QString installedDir();
};

#endif // DIRECTORYMANAGER_H
