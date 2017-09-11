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
    return boost::filesystem::system_complete(argv[0]).remove_filename();
}

boost::filesystem::path
OSXDirectoryManager::profileDir()
{
    return userDir() / "Library" / "Synergy";
}

boost::filesystem::path
OSXDirectoryManager::systemLogDir()
{
    return profileDir();
}
