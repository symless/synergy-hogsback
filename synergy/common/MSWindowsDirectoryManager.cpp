#define WIN32_LEAN_AND_MEAN
#include "MSWindowsDirectoryManager.h"
#include <Windows.h>
#include <Wtsapi32.h>
#include <shlobj.h>
#include <tchar.h>
#include <stdexcept>

boost::filesystem::path
MSWindowsDirectoryManager::systemAppDir()
{
    boost::filesystem::path dir;
    TCHAR result[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0,
                                  result))) {
        dir = result;
    } else {
        throw std::runtime_error ("Couldn't determine the system app data "
                                  "file path");
    }

    return dir / "Symless" / "Synergy";
}

boost::filesystem::path
MSWindowsDirectoryManager::installedDir()
{
    TCHAR exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    return boost::filesystem::path (exePath).parent_path();
}

boost::filesystem::path
MSWindowsDirectoryManager::profileDir()
{
    boost::filesystem::path dir;
    HANDLE sourceToken = NULL;
    WTSQueryUserToken(WTSGetActiveConsoleSessionId(), &sourceToken);

    /* When source is valid, we get the profile directory from that user. When
     * source is still NULL, we get profile directory from the user that spawned
     * this process
     */
    TCHAR result[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, sourceToken, 0,
                                  result))) {
        dir = result;
    } else {
        throw std::runtime_error ("Couldn't determine the users app data "
                                  "file path");
    }

    return dir / "Symless" / "Synergy";
}

boost::filesystem::path
MSWindowsDirectoryManager::systemLogDir()
{
    return systemAppDir();
}
