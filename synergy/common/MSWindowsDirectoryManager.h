#ifndef COMMONMSWINDOWSDIRECTORYMANAGER_H
#define COMMONMSWINDOWSDIRECTORYMANAGER_H

#include "DirectoryManager.h"

class MSWindowsDirectoryManager final : public DirectoryManager
{
public:
    boost::filesystem::path systemAppDir() override;
    boost::filesystem::path installedDir() override;
    boost::filesystem::path profileDir() override;
};

#endif // COMMONMSWINDOWSDIRECTORYMANAGER_H
