#ifndef COMMON_XWINDOWS_DIRECTORY_MANAGER_H
#define COMMON_XWINDOWS_DIRECTORY_MANAGER_H

#include "DirectoryManager.h"

class XWindowsDirectoryManager final : public DirectoryManager
{
public:
    boost::filesystem::path systemAppDir() override;
    boost::filesystem::path installedDir() override;
    boost::filesystem::path profileDir() override;
    boost::filesystem::path systemLogDir() override;
};

#endif // COMMON_XWINDOWS_DIRECTORY_MANAGER_H
