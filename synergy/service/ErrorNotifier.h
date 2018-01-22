#pragma once

#include <boost/signals2.hpp>

class CloudClient;
class ProfileConfig;
class CoreErrorMonitor;

class ErrorNotifier final {
public:
    explicit ErrorNotifier(CloudClient& cloudClient, ProfileConfig& profileConfig);

    void install(CoreErrorMonitor& monitor);

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

private:
    CloudClient& m_cloudClient;
    ProfileConfig& m_profileConfig;
};
