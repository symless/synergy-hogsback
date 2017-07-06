#include "XWindowsDirectoryManager.h"


std::string
XWindowsDirectoryManager::systemAppDir()
{
    return "/etc/synergy";
}

std::string
XWindowsDirectoryManager::installedDir()
{
    return "/usr/bin";
}

std::string
XWindowsDirectoryManager::profileDir()
{
    return userDir().append("/.synergy");
}
