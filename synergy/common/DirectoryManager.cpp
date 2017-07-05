#include "DirectoryManager.h"

#if SYSAPI_WIN32
#include "MSWindowsDirectoryManager.h"
#elif WINAPI_XWINDOWS
#include "XWindowsDirectoryManager.h"
#else
#include "OSXDirectoryManager.h"
#endif
DirectoryManager* DirectoryManager::s_instances = NULL;

DirectoryManager* DirectoryManager::instance()
{
    if (s_instances == NULL) {
#if SYSAPI_WIN32
#elif WINAPI_XWINDOWS
#else
        s_instances = new OSXDirectoryManager();
#endif

        return s_instances;
    }

    return s_instances;
}
