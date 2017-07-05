#include "GlobalConfig.h"

#include "ConfigParser.h"
#include "DirectoryManager.h"

#include <boost/filesystem.hpp>

const char* kGlobalConfigFilename = "synergy-system.cfg";

GlobalConfig::GlobalConfig()
{

}

std::string
GlobalConfig::filename()
{
    boost::filesystem::path dir(DirectoryManager::instance()->systemAppDir());
    boost::filesystem::path file(kGlobalConfigFilename);
    boost::filesystem::path filename = dir / file;
    return filename.string();
}

void GlobalConfig::load()
{
    ConfigParser parser = ConfigParser::parse_file(filename());
    m_lastProfileFilename = parser.get_value<std::string>("last-used-account.profile-dir");
    m_lastUserToken = parser.get_value<std::string>("last-used-account.user-token");
}
