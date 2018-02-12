#pragma once

#include <synergy/common/ProfileConfig.h>
#include <synergy/common/ProcessMode.h>
#include <synergy/service/router/ClientProxy.hpp>
#include <synergy/service/router/ServerProxy.hpp>

#include <boost/asio/steady_timer.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <string>

class Router;
class RpcManager;
class UserConfig;
class CoreProcess;
class CloudClient;
class ProcessCommand;
class ClaimMessageHandler;
class CoreErrorMonitor;
class CoreStatusMonitor;

class CoreManager final {
public:
    explicit CoreManager (boost::asio::io_service& io,
                             std::shared_ptr<UserConfig> userConfig,
                             std::shared_ptr<ProfileConfig> localProfileConfig,
                             std::shared_ptr<CloudClient> CloudClient,
                             RpcManager& rpc,
                             Router &router);

    CoreManager (CoreManager const&) = delete;
    CoreManager& operator= (CoreManager const&) = delete;
    ~CoreManager() noexcept;

    friend class ClaimMessageHandler;

    void shutdown();
    bool restart();
    bool setRunAsUid(std::string runAsUid);
    bool setDisplay(std::string);
    void switchServer(int64_t serverId);
    void notifyServerClaim(int64_t serverId);

    CoreErrorMonitor& errorMonitor() const;
    CoreStatusMonitor& statusMonitor() const;

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<UserConfig> m_userConfig;
    std::shared_ptr<ProfileConfig> m_localProfileConfig;
    std::unique_ptr<ClaimMessageHandler> m_messageHandler;
    std::shared_ptr<CloudClient> m_cloudClient;
    std::shared_ptr<ProcessCommand> m_processCommand;
    std::unique_ptr<CoreProcess> m_process;
    std::unique_ptr<CoreErrorMonitor> m_errorMonitor;
    RpcManager& m_rpc;
    Router& m_router;
    ServerProxy m_serverProxy;
    ClientProxy m_clientProxy;
    std::map<std::string, ScreenStatus> m_lastSeenStatus;
};
