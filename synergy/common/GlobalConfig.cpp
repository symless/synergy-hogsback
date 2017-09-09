#include "GlobalConfig.h"
#include "ConfigParser.h"
#include "DirectoryManager.h"
#include <boost/filesystem.hpp>

const char* const kGlobalConfigFilename = "synergy-system.cfg";

GlobalConfig::GlobalConfig()
{
}

std::string
GlobalConfig::filename()
{
    return (DirectoryManager::instance()->systemAppDir()
            / kGlobalConfigFilename).string();
}

void
GlobalConfig::load()
{
    ConfigParser parser = ConfigParser::parse_file(filename());
    m_lastProfileFilename = parser.get_value<std::string>("last-used-account.profile-dir");
    m_lastUserToken = parser.get_value<std::string>("last-used-account.user-token");
}
