#include "OSXDirectoryManager.h"

boost::filesystem::path
OSXDirectoryManager::systemAppDir()
{
    // TODO: use /Library instead?
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

boost::filesystem::path
OSXDirectoryManager::systemLogDir()
{
    return "/Library/Logs/Synergy";
}
