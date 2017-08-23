#ifndef COMMONUSERCONFIG_H
#define COMMONUSERCONFIG_H

#include "synergy/common/DebugLevel.h"

#include <string>

class  UserConfig final
{
public:
    UserConfig(std::string filename = "");

    std::string filename();

    void load();
    void save();

    DebugLevel debugLevel() const;
    void setDebugLevel(const DebugLevel &debugLevel);
    std::string userToken() const;
    void setUserToken(const std::string &userToken);
    int64_t userId() const;
    void setUserId(const int64_t &userId);
    int64_t profileId() const;
    void setProfileId(const int64_t &profileId);
    int64_t screenId() const;
    void setScreenId(const int64_t &screenId);
    bool dragAndDrop() const;
    void setDragAndDrop(bool dragAndDrop);

private:
    bool persistFilename();

private:
    std::string m_altFilename;
    DebugLevel m_debugLevel;
    std::string m_userToken;
    int64_t m_userId;
    int64_t m_profileId;
    int64_t m_screenId;
    bool m_dragAndDrop;
};

#endif // COMMONUSERCONFIG_H
