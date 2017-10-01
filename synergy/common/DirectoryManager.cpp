#include <synergy/common/DirectoryManager.h>

//#include <synergy/common/Logs.h>

#include <cstdlib>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#include <synergy/common/MSWindowsDirectoryManager.h>
#elif __APPLE__
#include <synergy/common/OSXDirectoryManager.h>
#else
#include <synergy/common/XWindowsDirectoryManager.h>
#endif

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

void
DirectoryManager::init(const std::string& argv0)
{
    m_argv0 = argv0;
    installDir();

    std::vector<boost::filesystem::path> paths;
    paths.push_back(userDir());
    paths.push_back(crashDumpDir());
    paths.push_back(systemAppDir());
    paths.push_back(profileDir());
    paths.push_back(systemLogDir());

    for (auto path : paths) {

        boost::system::error_code ec;
        if (!boost::filesystem::exists(path, ec)) {
            boost::filesystem::create_directories(path, ec);
        }

        if (ec) {
            throw boost::system::system_error(
                ec, "Failed to persist directory: " + path.string());
        }
    }
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
    return profileDir() / "dumps";
}
