#include "XWindowsDirectoryManager.h"

boost::filesystem::path
XWindowsDirectoryManager::systemAppDir()
{
    return "/etc/synergy";
}

boost::filesystem::path
XWindowsDirectoryManager::installedDir()
{
    return m_programDir;
}

boost::filesystem::path
XWindowsDirectoryManager::profileDir()
{
    return userDir() / ".config" / "Symless" / "Synergy";
}

boost::filesystem::path
XWindowsDirectoryManager::systemLogDir()
{
    return "/var/log";
}
