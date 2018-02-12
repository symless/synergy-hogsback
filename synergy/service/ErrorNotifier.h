#pragma once

#include <boost/signals2.hpp>

class CloudClient;
class ProfileConfig;
class CoreErrorMonitor;
class CoreStatusMonitor;
class RouterErrorMonitor;
class UserConfig;

class ErrorNotifier final {
public:
    explicit ErrorNotifier(CloudClient& cloudClient, ProfileConfig& profileConfig, UserConfig& userConfig);

    void install(CoreErrorMonitor& monitor);
    void install(CoreStatusMonitor& monitor);
    void install(RouterErrorMonitor& monitor);

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

private:
    CloudClient& m_cloudClient;
    ProfileConfig& m_profileConfig;
    UserConfig& m_userConfig;
};
