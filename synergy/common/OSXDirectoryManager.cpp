#include "OSXDirectoryManager.h"

boost::filesystem::path
OSXDirectoryManager::systemAppDir()
{
    return "/usr/local/share/synergy";
}

boost::filesystem::path
OSXDirectoryManager::installedDir()
{
    return "/Applications/Synergy.app/Contents/MacOS";
}

boost::filesystem::path
OSXDirectoryManager::profileDir()
{
    return userDir() / "Library" / "Synergy";
}
