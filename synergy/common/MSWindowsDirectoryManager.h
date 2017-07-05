#ifndef COMMONMSWINDOWSDIRECTORYMANAGER_H
#define COMMONMSWINDOWSDIRECTORYMANAGER_H

#include "DirectoryManager.h"

#include <string>

class MSWindowsDirectoryManager final : public DirectoryManager
{
public:
    std::string userDir();
    std::string systemAppDir();
    std::string installedDir();
    std::string profileDir();
};

#endif // COMMONMSWINDOWSDIRECTORYMANAGER_H
