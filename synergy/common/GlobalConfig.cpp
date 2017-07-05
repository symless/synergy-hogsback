#include "GlobalConfig.h"

#include "DirectoryManager.h"

#include <boost/filesystem.hpp>

const char* kGlobalConfigFilename = "synergy-system.cfg";

std::string
GlobalConfig::filename()
{
    boost::filesystem::path dir(DirectoryManager::instance()->systemAppDir());
    boost::filesystem::path file(kGlobalConfigFilename);
    boost::filesystem::path filename = dir / file;
    return filename.string();
}
