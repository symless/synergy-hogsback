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

    virtual QString profileDir() const;
    virtual QString crashDumpDir() const;
    virtual QString configFileDir() const;
    virtual QString installedDir() const;
};

#endif // DIRECTORYMANAGER_H
