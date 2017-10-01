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
    ssize_t size = readlink("/proc/self/exe", buffer, sizeof(buffer));
    if (size == 0 || size == sizeof(buffer)) {
        throw std::runtime_error("Could not ");
    }

    std::string path(buffer, size);

    boost::system::error_code ec;
    return boost::filesystem::canonical(
        path, boost::filesystem::current_path(), ec);
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
