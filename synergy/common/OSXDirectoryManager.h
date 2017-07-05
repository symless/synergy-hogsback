#ifndef COMMONDOSXIRECTORYMANAGER_H
#define COMMONDOSXIRECTORYMANAGER_H

#include "DirectoryManager.h"

#include <string>

class OSXDirectoryManager : public DirectoryManager
{
public:
    std::string systemAppDir();
};

#endif // COMMONDOSXIRECTORYMANAGER_H
