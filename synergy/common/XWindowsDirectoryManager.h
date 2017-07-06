#ifndef COMMON_XWINDOWS_DIRECTORY_MANAGER_H
#define COMMON_XWINDOWS_DIRECTORY_MANAGER_H

#include "DirectoryManager.h"

#include <string>

class XWindowsDirectoryManager final : public DirectoryManager
{
public:
    std::string systemAppDir();
    std::string installedDir();
    std::string profileDir();
};

#endif // COMMON_XWINDOWS_DIRECTORY_MANAGER_H
