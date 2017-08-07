#include "XWindowsDirectoryManager.h"


std::string
XWindowsDirectoryManager::systemAppDir()
{
    return "/etc/synergy";
}

boost::filesystem::path
XWindowsDirectoryManager::installedDir()
{
    return "/usr/bin";
}

std::string
XWindowsDirectoryManager::profileDir()
{
    return userDir().append("/.synergy");
}
