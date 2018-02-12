#pragma once

#include <synergy/common/ScreenError.h>

#include <boost/signals2.hpp>

class CoreProcess;

class CoreErrorMonitor final {
public:
    explicit CoreErrorMonitor(const std::string& localScreenName);

    void monitor(CoreProcess& process);

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(std::string const& screenName, ScreenError code, std::string const& message)> error;

private:
    std::string m_localScreenName;
};
