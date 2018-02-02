#ifndef COMMONUSERCONFIG_H
#define COMMONUSERCONFIG_H

#include <synergy/common/DebugLevel.h>
#include <boost/signals2/signal.hpp>
#include <string>
#include <cstdint>
#include <third_party/cpptoml/include/cpptoml.h>

class ConfigParser;

class UserConfig
{
public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    UserConfig();
    virtual ~UserConfig() = default;

    void reset();

    static std::string defaultFilePath();
    void load (std::istream& outputStream);
    void load (std::string const& filepath = defaultFilePath());
    void save (std::ostream& inputStream);
    void save (std::string const& filepath = defaultFilePath());

    /* Cloud config */
    std::string userToken() const;
    void setUserToken(const std::string& userToken);

    int64_t userId() const;
    void setUserId(const int64_t& userId);

    int64_t profileId() const;
    void setProfileId(const int64_t& profileId);

    virtual int64_t screenId() const;
    void setScreenId(const int64_t& screenId);

    /* Process options */
    DebugLevel debugLevel() const;
    void setDebugLevel(const DebugLevel& debugLevel);

    bool dragAndDrop() const;
    void setDragAndDrop(bool dragAndDrop);

    std::string systemUid() const;
    void setSystemUid(const std::string &systemUid);

    bool versionCheck() const;
    void setVersionCheck(bool versionCheck);

private:
    void makeTable(std::shared_ptr<cpptoml::table>& root);
    void update(ConfigParser& configParser);

private:
    /* Cloud config */
    std::string m_userToken;
    int64_t m_userId;
    int64_t m_profileId;
    int64_t m_screenId;
    std::string m_systemUid = "";
    bool m_hasDeveloperConfig = false;
    bool m_versionCheck = true;

    /* Process options */
    DebugLevel m_debugLevel = kInfo;
    bool m_dragAndDrop;

public:
    signal<void()> updated;
};

#endif // COMMONUSERCONFIG_H
