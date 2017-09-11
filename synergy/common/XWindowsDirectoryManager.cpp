#include "XWindowsDirectoryManager.h"

boost::filesystem::path
XWindowsDirectoryManager::systemAppDir()
{
    return "/etc/synergy";
}

boost::filesystem::path
XWindowsDirectoryManager::installedDir()
{
    return boost::filesystem::system_complete(argv[0]).remove_filename();
}

boost::filesystem::path
XWindowsDirectoryManager::profileDir()
{
    return userDir() / ".config" / "synergy";
}

boost::filesystem::path
XWindowsDirectoryManager::systemLogDir()
{
    return "/var/log";
}
