#ifndef COMMONDOSXIRECTORYMANAGER_H
#define COMMONDOSXIRECTORYMANAGER_H

#include "DirectoryManager.h"

#include <string>

class OSXDirectoryManager final : public DirectoryManager
{
public:
    std::string systemAppDir();
    boost::filesystem::path installedDir() override;
    std::string profileDir();
};

#endif // COMMONDOSXIRECTORYMANAGER_H
