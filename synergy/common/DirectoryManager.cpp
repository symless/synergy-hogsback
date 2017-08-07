#include "DirectoryManager.h"

#include <cstdlib>

#ifdef _WIN32
#include "MSWindowsDirectoryManager.h"
#elif __APPLE__
#include "OSXDirectoryManager.h"
#else
#include "XWindowsDirectoryManager.h"
#endif

DirectoryManager* DirectoryManager::s_instances = NULL;

DirectoryManager*
DirectoryManager::instance()
{
    if (s_instances == NULL) {
#ifdef _WIN32
        s_instances = new MSWindowsDirectoryManager();
#elif __APPLE__
        s_instances = new OSXDirectoryManager();
#else
        s_instances = new XWindowsDirectoryManager();
#endif
    }

    return s_instances;
}

std::string
DirectoryManager::userDir()
{
    const char* userDir = getenv("HOME");

    if (userDir) {
        return userDir;
    }

    throw;
}
