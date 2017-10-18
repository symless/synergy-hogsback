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
class CoreProcessImpl;

class CoreProcess final {
public:
    explicit CoreProcess (boost::asio::io_service& io,
                             std::shared_ptr<UserConfig> userConfig,
                             std::shared_ptr<ProfileConfig> localProfileConfig);
    CoreProcess (CoreProcess const&) = delete;
    CoreProcess& operator= (CoreProcess const&) = delete;
    ~CoreProcess() noexcept;

    void shutdown();

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void()> expectedExit;
    signal<void()> unexpectedExit;
    signal<void(const std::string& line)> output;
    signal<void()> localInputDetected;
    signal<void(const std::string& screenName)> screenConnectionError;
    signal<void(const std::string& screenName, ScreenStatus state)> screenStatusChanged;
    void setRunAsUid(const std::string& runAsUid);

    int currentServerId() const;
    ProcessMode proccessMode() const;

private:
    void onServerChanged(int64_t serverId);
    void start (std::vector<std::string> command);
    void writeConfigurationFile();
    void startServer();
    void startClient(int serverId);

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<UserConfig> m_userConfig;
    std::shared_ptr<ProfileConfig> m_localProfileConfig;
    std::unique_ptr<CoreProcessImpl> m_impl;
    ProcessMode m_proccessMode;
    int m_currentServerId;
    std::vector<std::string> m_nextCommand;
    std::string m_runAsUid = "";
};

#endif // SYNERGY_SERVICE_PROCESSMANAGER_H
