#pragma once

#include <synergy/common/ProfileConfig.h>
#include <synergy/common/ProcessMode.h>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <memory>
#include <vector>
#include <string>
#include <boost/asio/steady_timer.hpp>

class UserConfig;
class ProcessCommand;
class CoreProcessImpl;
class CoreStatusMonitor;

class CoreProcess final {
public:
    explicit CoreProcess (boost::asio::io_service& io,
                             std::shared_ptr<UserConfig> userConfig,
                             std::shared_ptr<ProfileConfig> localProfileConfig,
                             std::shared_ptr<ProcessCommand> processCommand);

    CoreProcess (CoreProcess const&) = delete;
    CoreProcess& operator= (CoreProcess const&) = delete;
    ~CoreProcess() noexcept;

    int currentServerId() const;
    ProcessMode processMode() const;
    void start (std::vector<std::string> command);
    void shutdown();
    void startServer();
    void startClient(int serverId);
    CoreStatusMonitor& statusMonitor() const;

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void()> expectedExit;
    signal<void()> unexpectedExit;
    signal<void()> localInputDetected;
    signal<void()> serverReady;
    signal<void(std::string const& line)> output;
    signal<void(std::string const& screenName)> screenConnectionError;

private:
    void writeConfigurationFile();

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<UserConfig> m_userConfig;
    std::shared_ptr<ProfileConfig> m_localProfileConfig;
    std::unique_ptr<CoreProcessImpl> m_impl;
    boost::asio::steady_timer m_retryTimer;
    ProcessMode m_processMode;
    int m_currentServerId;
    std::vector<std::string> m_nextCommand;
    std::vector<std::string> m_lastCommand;
    std::shared_ptr<ProcessCommand> m_processCommand;
    std::unique_ptr<CoreStatusMonitor> m_statusMonitor;
};
