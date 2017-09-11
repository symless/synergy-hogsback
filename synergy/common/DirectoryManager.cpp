#include "DirectoryManager.h"

#include <cstdlib>
#include <stdexcept>

#ifdef _WIN32
#include "MSWindowsDirectoryManager.h"
#elif __APPLE__
#include "OSXDirectoryManager.h"
#else
#include "XWindowsDirectoryManager.h"
#endif

std::string g_programDir;

DirectoryManager*
DirectoryManager::instance()
{
#ifdef _WIN32
    static auto impl = new MSWindowsDirectoryManager();
#elif __APPLE__
    static auto impl = new OSXDirectoryManager();
#else
    static auto impl = new XWindowsDirectoryManager();
#endif
    return impl;
}

boost::filesystem::path
DirectoryManager::userDir()
{
    const char* userDir = getenv("HOME");
    if (!userDir) {
        throw std::runtime_error("HOME environment variable not set");
    }
    return userDir;
}

boost::filesystem::path
DirectoryManager::crashDumpDir() {
    auto path = profileDir() / "dumps";
    boost::system::error_code ec;
    if (!boost::filesystem::exists (path, ec) && !ec) {
        boost::filesystem::create_directories (path, ec);
    }
    if (ec) {
        throw boost::system::system_error(ec, "Couldn't verify crash dump "
                                              "directory");
    }
    return path;
}
