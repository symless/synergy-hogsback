#include "UserConfig.h"

#include "ConfigParser.h"
#include "DirectoryManager.h"

#include <boost/filesystem.hpp>
#include <cpptoml.h>
#include <iostream>
#include <fstream>

static const char* const kUserConfigFilename = "synergy-user.cfg";

UserConfig::UserConfig():
    m_userToken(),
    m_userId(-1),
    m_profileId(-1),
    m_screenId(-1),
    m_debugLevel(kInfo),
    m_dragAndDrop(false)
{
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
UserConfig::load(std::string const& filepath)
{
    ConfigParser parser = ConfigParser::parse_file(filepath);

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
}

void UserConfig::save(std::string const& filepath)
{
    std::shared_ptr<cpptoml::table> root = cpptoml::make_table();

    auto authTable = cpptoml::make_table();
    authTable->insert("user-id", m_userId);
    authTable->insert("user-token", m_userToken);

    auto profileTable = cpptoml::make_table();
    profileTable->insert("profile-id", m_profileId);
    profileTable->insert("screen-id", m_screenId);
    profileTable->insert("drag-and-drop", m_dragAndDrop);
    profileTable->insert("debug-level", (int)m_debugLevel);

    root->insert("auth", authTable);
    root->insert("profile", profileTable);

    std::ofstream file;
    file.imbue (std::locale::classic());
    file.open (filepath, std::ios::trunc | std::ios::out);
    file << *root;
    file.close();
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
