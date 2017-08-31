#include "MSWindowsDirectoryManager.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Wtsapi32.h>
#include <shlobj.h>
#include <tchar.h>
#include <string.h>

std::string
MSWindowsDirectoryManager::systemAppDir()
{
    std::string dir;
    TCHAR result[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, result))) {
        dir = result;
    }

    return dir + "\\Symless\\Synergy";
}

boost::filesystem::path
MSWindowsDirectoryManager::installedDir()
{
    char fileNameBuffer[MAX_PATH];
    GetModuleFileName(NULL, fileNameBuffer, MAX_PATH);
    return boost::filesystem::path (fileNameBuffer).parent_path().string();
}

std::string
MSWindowsDirectoryManager::profileDir()
{
    std::string dir;
    HANDLE sourceToken = NULL;
    WTSQueryUserToken(WTSGetActiveConsoleSessionId(), &sourceToken);

    // when source is valid, we get the profile directory from that user
    // when source is still NULL, we get profile directory from the user
    // that spawns this process
    TCHAR result[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, sourceToken, 0, result))) {
        dir = result;
    }
    else {
        throw;
    }

    dir.append("\\Symless\\Synergy");
    return dir;
}

std::string MSWindowsDirectoryManager::pathSeparator()
{
    return "\\";
}
