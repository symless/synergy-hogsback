#include "OSXDirectoryManager.h"

boost::filesystem::path
OSXDirectoryManager::systemAppDir()
{
    // TODO: use /Library instead?
    return "/usr/local/share/synergy";
}

boost::filesystem::path
OSXDirectoryManager::installDir()
{
    auto path = boost::filesystem::system_complete(argv0());
    return path.remove_filename().string();
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
