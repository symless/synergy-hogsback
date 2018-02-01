#include "UserConfig.h"

#include "ConfigParser.h"
#include "DirectoryManager.h"

#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>

static const char* const kUserConfigFilename = "synergy-user.cfg";

UserConfig::UserConfig()
{
    reset();
}

void UserConfig::reset()
{
    m_userToken.clear();
    m_userId = -1;
    m_profileId = -1;
    m_screenId = -1;
    m_debugLevel = kInfo;
    m_dragAndDrop = false;
}

std::string
UserConfig::defaultFilePath()
{
#ifdef _WIN32
    auto filepath = DirectoryManager::instance()->systemAppDir();
#else
    auto filepath = DirectoryManager::instance()->profileDir();
#endif
    filepath /= kUserConfigFilename;
    return filepath.string();
}

void
UserConfig::load(std::istream& stream)
{
    ConfigParser parser = ConfigParser::parse_memory(stream);
    update(parser);
}

void
UserConfig::load(std::string const& filepath)
{
    if (!boost::filesystem::exists(filepath)) {
        return;
    }

    ConfigParser parser = ConfigParser::parse_file(filepath);
    update(parser);
}

void
UserConfig::update(ConfigParser& parser)
{
    auto authConfig = parser.get_section("auth");
    if (authConfig.isValid()) {
        m_userId = authConfig.get_value<int64_t>("user-id");
        m_userToken = authConfig.get_value<std::string>("user-token");
    }

    auto profileConfig = parser.get_section("profile");
    if (profileConfig.isValid()) {
        m_profileId = profileConfig.get_value<int64_t>("profile-id");
        m_screenId = profileConfig.get_value<int64_t>("screen-id");
        m_dragAndDrop = profileConfig.get_value<bool>("drag-and-drop");
        m_debugLevel = static_cast<DebugLevel>(profileConfig.get_value<int64_t>("debug-level"));
    }

    auto systemConfig = parser.get_section("system");
    if (systemConfig.isValid()) {
        m_systemUid = systemConfig.get_value<std::string>("uid");
    }

    auto developerConfig = parser.get_section("developer");
    m_hasDeveloperConfig = developerConfig.isValid();
    if (m_hasDeveloperConfig) {
        m_versionCheck = developerConfig.get_value<bool>("version-check");
    }

    updated();
}

void
UserConfig::makeTable(std::shared_ptr<cpptoml::table>& root)
{
    auto authTable = cpptoml::make_table();
    authTable->insert("user-id", m_userId);
    authTable->insert("user-token", m_userToken);
    root->insert("auth", authTable);

    auto profileTable = cpptoml::make_table();
    profileTable->insert("profile-id", m_profileId);
    profileTable->insert("screen-id", m_screenId);
    profileTable->insert("drag-and-drop", m_dragAndDrop);
    profileTable->insert("debug-level", (int)m_debugLevel);
    root->insert("profile", profileTable);

    auto systemTable = cpptoml::make_table();
    systemTable->insert("uid", m_systemUid);
    root->insert("system", systemTable);

    // only save developer section if there was one when we read
    // the config, which hides developer options from end-users
    if (m_hasDeveloperConfig) {
        auto developerTable = cpptoml::make_table();
        developerTable->insert("version-check", m_versionCheck);
        root->insert("developer", developerTable);
    }
}

void
UserConfig::save(std::ostream& inputStream)
{
    std::shared_ptr<cpptoml::table> root = cpptoml::make_table();
    makeTable(root);

    inputStream.imbue (std::locale::classic());
    inputStream << *root;

    updated();
}

void
UserConfig::save(std::string const& filepath)
{
    std::shared_ptr<cpptoml::table> root = cpptoml::make_table();
    makeTable(root);

    std::ofstream file;
    file.imbue (std::locale::classic());
    file.open (filepath, std::ios::trunc | std::ios::out);
    file << *root;
    file.close();

    updated();
}

DebugLevel UserConfig::debugLevel() const
{
    return m_debugLevel;
}

void UserConfig::setDebugLevel(const DebugLevel &debugLevel)
{
    m_debugLevel = debugLevel;
}

std::string UserConfig::userToken() const
{
    return m_userToken;
}

void UserConfig::setUserToken(const std::string &userToken)
{
    m_userToken = userToken;
}

int64_t UserConfig::userId() const
{
    return m_userId;
}

void UserConfig::setUserId(const int64_t &userId)
{
    m_userId = userId;
}

int64_t UserConfig::profileId() const
{
    return m_profileId;
}

void UserConfig::setProfileId(const int64_t &profileId)
{
    m_profileId = profileId;
}

int64_t UserConfig::screenId() const
{
    return m_screenId;
}

void UserConfig::setScreenId(const int64_t &screenId)
{
    m_screenId = screenId;
}

bool UserConfig::dragAndDrop() const
{
    return m_dragAndDrop;
}

void UserConfig::setDragAndDrop(bool dragAndDrop)
{
    m_dragAndDrop = dragAndDrop;
}

bool UserConfig::versionCheck() const
{
    return m_versionCheck;
}

void UserConfig::setVersionCheck(bool versionCheck)
{
    m_versionCheck = versionCheck;
}

std::string UserConfig::systemUid() const
{
    return m_systemUid;
}

void UserConfig::setSystemUid(const std::string &systemUid)
{
    m_systemUid = systemUid;
}
