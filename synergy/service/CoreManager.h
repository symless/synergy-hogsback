#pragma once

#include <synergy/common/ProfileConfig.h>
#include <synergy/common/ProcessMode.h>


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
    void setRunAsUid(const std::string& runAsUid);
    void switchServer(int64_t serverId);
    void notifyServerClaim(int64_t serverId);

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<UserConfig> m_userConfig;
    std::shared_ptr<ProfileConfig> m_localProfileConfig;
    std::unique_ptr<ClaimMessageHandler> m_messageHandler;
    std::shared_ptr<CloudClient> m_cloudClient;
    std::shared_ptr<ProcessCommand> m_processCommand;
    std::unique_ptr<CoreProcess> m_process;
    RpcManager& m_rpc;
    Router& m_router;
};
