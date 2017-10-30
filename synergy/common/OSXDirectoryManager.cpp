#include "OSXDirectoryManager.h"

boost::filesystem::path
OSXDirectoryManager::systemAppDir()
{
    // on mac, everything runs as user (not root), so use
    // a dir that the user always has write access to.
    // this seems safe, as of writing this comment only
    // GlobalConfig uses this dir.
    return profileDir();
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
