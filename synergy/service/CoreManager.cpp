#include <synergy/service/CoreManager.h>

#include <synergy/common/ProcessCommand.h>
#include <synergy/common/RpcManager.h>
#include <synergy/common/WampServer.h>
#include <synergy/common/UserConfig.h>
#include <synergy/service/ServiceLogs.h>
#include <synergy/service/CoreProcess.h>
#include <synergy/service/CloudClient.h>
#include <synergy/service/router/protocol/v2/MessageTypes.hpp>
#include <synergy/service/router/Router.hpp>
#include <synergy/common/NetworkParameters.h>

#include <cassert>
#include <vector>
#include <string>

class ClaimMessageHandler final {
public:
    explicit ClaimMessageHandler (CoreManager& coreManager) :
        m_coreManager(coreManager)
    {
    }

    void operator() (Message const&, std::uint32_t source) const;
    void handle (ServerClaim const&, std::uint32_t source) const;

    template <typename T>
    void handle (T const&,  std::uint32_t) const;

private:
    CoreManager& m_coreManager;
};

void
ClaimMessageHandler::operator() (Message const& message,
                                       std::uint32_t const source) const {
    boost::apply_visitor (
        [this, source](auto& body) { this->handle (body, source); },
        message.body()
    );
}

void
ClaimMessageHandler::handle (const ServerClaim &msg,
                                   std::uint32_t) const {
    auto userConfig = m_coreManager.m_userConfig;
    auto& process = *(m_coreManager.m_process);

    if (msg.profile_id != userConfig->profileId()) {
        serviceLog()->debug ("ignore a server claim message from a different profile, current profile:{}, calim message in profile:{}",
                             userConfig->profileId(), msg.profile_id);
        return;
    }

    serviceLog()->debug("handling router message: server claim, mode={} thisId={} serverId={} lastServerId={}",
        processModeToString(process.processMode()), userConfig->screenId(), msg.screen_id, process.currentServerId());

    m_coreManager.switchServer(msg.screen_id);
}

template <typename T> inline
void
ClaimMessageHandler::handle (T const&, std::uint32_t) const {
}

CoreManager::CoreManager (boost::asio::io_service& io,
                          std::shared_ptr<UserConfig> userConfig,
                          std::shared_ptr<ProfileConfig> localProfileConfig,
                          std::shared_ptr<CloudClient> CloudClient, RpcManager &rpc,
                          Router& router) :
    m_ioService (io),
    m_userConfig (std::move (userConfig)),
    m_localProfileConfig (std::move (localProfileConfig)),
    m_cloudClient (CloudClient),
    m_processCommand (std::make_shared<ProcessCommand>()),
    m_process (std::make_unique<CoreProcess>(m_ioService, m_userConfig, m_localProfileConfig, m_processCommand)),
    m_rpc (rpc),
    m_router (router),
    m_serverProxy (io, m_router, kServerProxyPort),
    m_clientProxy (io, m_router, kServerPort)
{

    m_processCommand->setLocalHostname(boost::asio::ip::host_name());

    m_messageHandler = std::make_unique<ClaimMessageHandler> (*this);
    m_router.on_receive.connect (*m_messageHandler);

    m_process->localInputDetected.connect([this](){
        m_ioService.post([this] () {
            auto screenId = m_userConfig->screenId();
            switchServer(screenId);
            notifyServerClaim(screenId);
        });

        serviceLog()->debug("local input detected, claiming this computer as server in local network");
    });

    m_process->serverReady.connect([this](){
        m_cloudClient->claimServer();
    });

    auto server = m_rpc.server();
    m_process->output.connect(
        [server](std::string line) {
            coreLog()->debug(line);
            server->publish ("synergy.core.log", std::move(line));
        }
    );

    m_process->screenStatusChanged.connect(
        [server](std::string const& screenName, ScreenStatus state) {
            server->publish ("synergy.screen.status", screenName, int(state));
        }
    );

    m_process->screenConnectionError.connect(
        [server](std::string const& screenName) {

            // TODO: get error code
            int ec = 0;
            server->publish ("synergy.screen.error", screenName,
                                (int)ec);
            server->publish ("synergy.screen.status", screenName,
                                (int)ScreenStatus::kConnectingWithError);
        }
    );

    m_localProfileConfig->profileServerChanged.connect([this](int64_t const serverId) {
        m_ioService.post([this, serverId] () {
            serviceLog()->debug("handling cloud message: server claim, mode={} thisId={} serverId={} lastServerId={}",
                processModeToString(m_process->processMode()), m_userConfig->screenId(), serverId, m_process->currentServerId());

            switchServer(serverId);
        });
    });

    m_localProfileConfig->screenPositionChanged.connect([this](int64_t const /* targetScreenID */){
        m_ioService.post([this]() {
            auto processMode = m_process->processMode();
            serviceLog()->debug ("restarting core because local profile screen "
                                 "position changed, mode={}",
                                 processModeToString(processMode));
            if (processMode == ProcessMode::kServer) {
                m_process->startServer();

                // HACK: send server claim in local network
                // reason: when there is a screen position change, server needs to restart itself
                // on the client side, it relis on connection timeout to reconnect to the new server
                // sending this local claim will trigger clients to reconnect immediately
                notifyServerClaim(m_userConfig->screenId());
            }
        });
    });

    m_localProfileConfig->screenSetChanged.connect([this](std::vector<Screen> const&,
                                                          std::vector<Screen> const&) {
        m_ioService.post([this]() {
            auto processMode = m_process->processMode();

            serviceLog()->debug ("restarting core because local profile screen "
                                 "set changed, mode={}",
                                 processModeToString(processMode));
            if (processMode == ProcessMode::kServer) {
                m_process->startServer();

                // HACK: send server claim in local network
                // reason: when there is a new screen added or removed, server needs to restart itself
                // on the client side, it relis on connection timeout to reconnect to the new server
                // sending this local claim will trigger clients to reconnect immediately
                notifyServerClaim(m_userConfig->screenId());
            }
        });
    });

    if (m_userConfig->screenId() != -1) {
        m_router.start (m_userConfig->screenId(), boost::asio::ip::host_name());
        m_clientProxy.start ();
    }
    else {
        m_userConfig->updated.connect_extended ([this](const auto& connection) {
            if (m_userConfig->screenId() != -1) {
                m_router.start (m_userConfig->screenId(), boost::asio::ip::host_name());
                m_clientProxy.start ();
                connection.disconnect();
            }
        });
    }
}

CoreManager::~CoreManager () noexcept {
    shutdown();
}

void
CoreManager::shutdown() {
    m_process->shutdown();
}

void
CoreManager::setRunAsUid(const std::string &runAsUid)
{
    m_processCommand->setRunAsUid(runAsUid);
}

void
CoreManager::switchServer(int64_t serverId)
{
    switch (m_process->processMode()) {
        case ProcessMode::kServer: {
            // when server changes from local screen to another screen
            if (m_userConfig->screenId() != serverId) {
                m_serverProxy.start(serverId);
                m_process->startClient(serverId);
            }
            else {
                serviceLog()->debug("core is already in server mode, ignoring switch");
            }

            break;
        }
        case ProcessMode::kClient: {
            // when local screen becomes the server
            if (m_userConfig->screenId() == serverId) {
                m_process->startServer();
                break;
            }

            // when another screen, not local screen, claims to be the server
            m_serverProxy.start(serverId);
            m_process->startClient(serverId);

            break;
        }
        case ProcessMode::kUnknown: {
            if (m_userConfig->screenId() == serverId) {
                m_process->startServer();
            }
            else {
                m_serverProxy.start(serverId);
                m_process->startClient(serverId);
            }

            break;
        }
    }
}

void
CoreManager::notifyServerClaim(int64_t serverId)
{
    ServerClaim serverClaimMessage;
    serverClaimMessage.profile_id = m_userConfig->profileId();
    serverClaimMessage.screen_id = serverId;
    Message message(serverClaimMessage);

    m_router.notifyOtherNodes(message);
}
