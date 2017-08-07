#include "OSXDirectoryManager.h"


std::string
OSXDirectoryManager::systemAppDir()
{
    return "/usr/local/share";
}

boost::filesystem::path
OSXDirectoryManager::installedDir()
{
    return "/Applications/Synergy.app/Contents/MacOS";
}

std::string
OSXDirectoryManager::profileDir()
{
    return userDir().append("/Library/Synergy");
}
