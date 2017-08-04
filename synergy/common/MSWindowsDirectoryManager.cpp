#include "MSWindowsDirectoryManager.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj.h>
#include <tchar.h>
#include <string.h>

std::string
MSWindowsDirectoryManager::userDir()
{
    // try %HOMEPATH%
    TCHAR dir[MAX_PATH];
    DWORD size   = sizeof(dir) / sizeof(TCHAR);
    DWORD result = GetEnvironmentVariable(_T("HOMEPATH"), dir, size);
    if (result != 0 && result <= size) {
        // sanity check -- if dir doesn't appear to start with a
        // drive letter and isn't a UNC name then don't use it
        // FIXME -- allow UNC names
        if (dir[0] != '\0' && (dir[1] == ':' ||
            ((dir[0] == '\\' || dir[0] == '/') &&
            (dir[1] == '\\' || dir[1] == '/')))) {
            return dir;
        }
    }

    // get the location of the personal files.  that's as close to
    // a home directory as we're likely to find.
    ITEMIDLIST* idl;
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &idl))) {
        TCHAR* path = NULL;
        if (SHGetPathFromIDList(idl, dir)) {
            DWORD attr = GetFileAttributes(dir);
            if (attr != 0xffffffff && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
                path = dir;
        }

        IMalloc* shalloc;
        if (SUCCEEDED(SHGetMalloc(&shalloc))) {
            shalloc->Free(idl);
            shalloc->Release();
        }

        if (path != NULL) {
            return path;
        }
    }

    // use root of C drive as a default
    return "C:";
}

std::string
MSWindowsDirectoryManager::systemAppDir()
{
    return "";
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
    TCHAR result[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, result))) {
        dir = result;
    }
    else {
        dir = userDir();
    }

    dir.append("\\Synergy");

    return dir;
}

std::string MSWindowsDirectoryManager::pathSeparator()
{
    return "\\";
}
