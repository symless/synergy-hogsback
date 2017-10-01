#ifndef COMMONMSWINDOWSDIRECTORYMANAGER_H
#define COMMONMSWINDOWSDIRECTORYMANAGER_H

#include "DirectoryManager.h"

class MSWindowsDirectoryManager final : public DirectoryManager
{
public:
    boost::filesystem::path systemAppDir() override;
    boost::filesystem::path installDir() override;
    boost::filesystem::path profileDir() override;
    boost::filesystem::path systemLogDir() override;
};

#endif // COMMONMSWINDOWSDIRECTORYMANAGER_H
