#ifndef SYNERGY_SERVICE_PROCESSMANAGER_H
#define SYNERGY_SERVICE_PROCESSMANAGER_H

#include <synergy/common/ProfileConfig.h>
#include <synergy/common/ProcessMode.h>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <memory>
#include <vector>
#include <string>

class Screen;
class UserConfig;
class ProcessManagerImpl;
class ConnectivityTester;

class ProcessManager final {
public:
    explicit ProcessManager (boost::asio::io_service& io,
                             std::shared_ptr<UserConfig> userConfig,
                             std::shared_ptr<ProfileConfig> localProfileConfig);
    ProcessManager (ProcessManager const&) = delete;
    ProcessManager& operator= (ProcessManager const&) = delete;
    ~ProcessManager() noexcept;

    void start (std::vector<std::string> command);
    void shutdown();

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void()> onExit;
    signal<void()> onUnexpectedExit;
    signal<void(std::string const&)> onOutput;
    signal<void()> onLocalInput;

private:
    void writeConfigurationFile();
    void startServer();
    void startClient(int serverId);

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<UserConfig> m_userConfig;
    std::shared_ptr<ProfileConfig> m_localProfileConfig;
    std::unique_ptr<ProcessManagerImpl> m_impl;
    std::unique_ptr<ConnectivityTester> m_connectivityTester;
    ProcessMode m_proccessMode;
    int m_lastServerId;
};

#endif // SYNERGY_SERVICE_PROCESSMANAGER_H
