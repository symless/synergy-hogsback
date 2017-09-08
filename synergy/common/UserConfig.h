#ifndef COMMONUSERCONFIG_H
#define COMMONUSERCONFIG_H

#include <synergy/common/DebugLevel.h>
#include <string>
#include <cstdint>

class UserConfig final
{
public:
    UserConfig();
    static std::string defaultFilePath();
    void load (std::string const& filepath = defaultFilePath());
    void save (std::string const& filepath = defaultFilePath());

    /* Cloud config */
    std::string userToken() const;
    void setUserToken(const std::string& userToken);

    int64_t userId() const;
    void setUserId(const int64_t& userId);

    int64_t profileId() const;
    void setProfileId(const int64_t& profileId);

    int64_t screenId() const;
    void setScreenId(const int64_t& screenId);

    /* Process options */
    DebugLevel debugLevel() const;
    void setDebugLevel(const DebugLevel& debugLevel);

    bool dragAndDrop() const;
    void setDragAndDrop(bool dragAndDrop);

private:
    /* Cloud config */
    std::string m_userToken;
    int64_t m_userId;
    int64_t m_profileId;
    int64_t m_screenId;

    /* Process options */
    DebugLevel m_debugLevel = kInfo;
    bool m_dragAndDrop;
};

#endif // COMMONUSERCONFIG_H
