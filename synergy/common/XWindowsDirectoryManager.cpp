#include "XWindowsDirectoryManager.h"

boost::filesystem::path
XWindowsDirectoryManager::systemAppDir()
{
    return "/etc/synergy";
}

boost::filesystem::path
XWindowsDirectoryManager::installDir()
{
    char buffer[1024] = {0};
    const char* processFile = "/proc/self/exe";
    ssize_t size = readlink(processFile, buffer, sizeof(buffer));
    if (size == 0 || size == sizeof(buffer)) {
        throw std::runtime_error("Could not read process info from: " + std::string(processFile));
    }

    std::string pathString(buffer, size);

    boost::system::error_code ec;
    auto path = boost::filesystem::canonical(
        pathString, boost::filesystem::current_path(), ec);
    return path.remove_filename().string();
}

boost::filesystem::path
XWindowsDirectoryManager::profileDir()
{
    /* we can't use userDir() on linux, because we run as a root
     * service which doesn't have a home dir, so use etc instead.
     *
     * TODO: andrew said "this is solved by having the service
     * remember the last user id"
     */
    //return userDir() / ".config" / "Symless" / "Synergy";
    return "/var/lib/synergy";
}

boost::filesystem::path
XWindowsDirectoryManager::systemLogDir()
{
    return "/var/log/synergy";
}
