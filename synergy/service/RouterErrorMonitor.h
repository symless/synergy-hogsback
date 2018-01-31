#pragma once

#include <boost/signals2.hpp>
#include <map>

class Router;
class ProfileConfig;

class RouterErrorMonitor final {
public:
    explicit RouterErrorMonitor(std::shared_ptr<ProfileConfig> localProfileConfig);

    void monitor(Router& router);

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(int64_t screen_id)> screenDiscovered;
    signal<void(int64_t screen_id)> screenLost;

private:
    void add(int64_t screenId, std::string Ip);
    void remove(int64_t screenId);

private:
    std::multimap<int64_t, std::string> m_monitoringScreenIp;
    std::shared_ptr<ProfileConfig> m_localProfileConfig;
};
