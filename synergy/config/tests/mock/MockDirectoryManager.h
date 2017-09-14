#ifndef MOCKDIRECTORYMANAGER_H
#define MOCKDIRECTORYMANAGER_H

#include "DirectoryManager.h"

class MockDirectoryManager : public GUIDirectoryManager
{
public:
    MockDirectoryManager();

    QString profileDir();
    QString configFileDir();
    QString installedDir();
};

#endif // MOCKDIRECTORYMANAGER_H
