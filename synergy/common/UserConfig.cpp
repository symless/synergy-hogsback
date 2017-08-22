#include "UserConfig.h"

#include "DirectoryManager.h"

const char* kUserConfigFilename = "synergy-user.cfg";

std::string
UserConfig::filename()
{
    boost::filesystem::path dir(DirectoryManager::instance()->profileDir());
    boost::filesystem::path file(kUserConfigFilename);
    boost::filesystem::path filename = dir / file;
    return filename.string();
}

void UserConfig::load()
{

}

void UserConfig::save()
{

}
