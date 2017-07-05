#ifndef COMMONMSWINDOWSDIRECTORYMANAGER_H
#define COMMONMSWINDOWSDIRECTORYMANAGER_H

#include "DirectoryManager.h"

#include <string>

class MSWindowsDirectoryManager : public DirectoryManager
{
public:
    std::string systemAppDir();
};

#endif // COMMONMSWINDOWSDIRECTORYMANAGER_H
