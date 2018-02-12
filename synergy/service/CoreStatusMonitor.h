#pragma once

#include <synergy/common/UserConfig.h>
#include <synergy/common/ProfileConfig.h>
#include <synergy/common/ScreenStatus.h>

#include <boost/signals2.hpp>
#include <map>
#include <vector>
#include <string>

class CoreProcess;

class CoreStatusMonitor final {
public:
    explicit CoreStatusMonitor(std::shared_ptr<UserConfig> userConfig, std::shared_ptr<ProfileConfig> localProfileConfig);

    void monitor(CoreProcess& process);

    void update(const std::string& screenName, ScreenStatus status);

private:
    void reset();

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(std::string const& screenName, ScreenStatus status)> screenStatusChanged;

private:
    std::vector<boost::signals2::connection> m_signals;
    std::map<std::string, ScreenStatus> m_screenStates;
    std::shared_ptr<UserConfig> m_userConfig;
    std::shared_ptr<ProfileConfig> m_localProfileConfig;
};
