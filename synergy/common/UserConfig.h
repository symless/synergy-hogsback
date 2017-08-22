#ifndef COMMONUSERCONFIG_H
#define COMMONUSERCONFIG_H

#include "synergy/common/DebugLevel.h"

#include <string>

class  UserConfig
{
public:
    std::string filename();
    void load();
    void save();

private:
    DebugLevel m_debugLevel;
    std::string m_localIp;
    std::string m_userToken;
    int m_userId;
    int m_profileId;
    int m_screenId;
    bool m_dragAndDrop;
};

#endif // COMMONUSERCONFIG_H
