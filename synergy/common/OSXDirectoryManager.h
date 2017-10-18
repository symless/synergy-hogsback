#ifndef COMMONDOSXIRECTORYMANAGER_H
#define COMMONDOSXIRECTORYMANAGER_H

#include "DirectoryManager.h"

class OSXDirectoryManager final : public DirectoryManager
{
public:
    boost::filesystem::path systemAppDir() override;
    boost::filesystem::path installDir() override;
    boost::filesystem::path profileDir() override;
    boost::filesystem::path systemLogDir() override;
};

#endif // COMMONDOSXIRECTORYMANAGER_H
