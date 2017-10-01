#ifndef DIRECTORYMANAGER_H
#define DIRECTORYMANAGER_H

#include "LibMacro.h"

#include <QObject>

class LIB_SPEC GUIDirectoryManager : QObject
{
	Q_OBJECT

public:
    GUIDirectoryManager();
    virtual ~GUIDirectoryManager();

    virtual QString profileDir() const;
    virtual QString crashDumpDir() const;
    virtual QString configFileDir() const;
    virtual QString installDir() const;
};

#endif // DIRECTORYMANAGER_H
