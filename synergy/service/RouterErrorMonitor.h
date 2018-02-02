#pragma once

#include <boost/signals2.hpp>
#include <memory>
#include <vector>

class Router;
class ProfileConfig;
struct RouterErrorScreenMonitor;

class RouterErrorMonitor final {
public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

public:
    explicit RouterErrorMonitor (std::shared_ptr<ProfileConfig> localProfileConfig,
                                Router& router);
    ~RouterErrorMonitor();

    signal<void(int64_t)> screenReachable;
    signal<void(int64_t)> screenUnreachable;

private:
    std::shared_ptr<ProfileConfig> m_localProfileConfig;
    std::vector<std::unique_ptr<RouterErrorScreenMonitor>> m_monitors;
};
