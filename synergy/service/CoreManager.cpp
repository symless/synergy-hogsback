#include <synergy/service/CoreManager.h>

#include <synergy/common/ProcessCommand.h>
#include <synergy/common/RpcManager.h>
#include <synergy/common/WampServer.h>
#include <synergy/common/UserConfig.h>
#include <synergy/service/ServiceLogs.h>
#include <synergy/service/CoreProcess.h>
#include <synergy/service/CoreStatusMonitor.h>
#include <synergy/service/CoreErrorMonitor.h>
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
    // TODO: unify hostname between UI and service
    m_errorMonitor(std::make_unique<CoreErrorMonitor>(boost::asio::ip::host_name())),
    m_rpc (rpc),
    m_router (router),
    m_serverProxy (io, m_router, kServerProxyPort),
    m_clientProxy (io, m_router, kServerPort)
{
    // TODO: unify hostname between UI and service
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
        m_localProfileConfig->claimServer(m_userConfig->screenId());
    });

    auto server = m_rpc.server();
    m_process->output.connect(
        [server](std::string line) {
            coreLog()->debug(line);
            server->publish ("synergy.core.log", std::move(line));
        }
    );

    m_process->statusMonitor().screenStatusChanged.connect(
        [server, this](std::string const& screenName, ScreenStatus status) {
            server->publish ("synergy.screen.status", screenName, int(status));

            // HACK: use a status record
            // reason: currently we still relying on UI to send status update
            // so we can't garantee always receive the lastest snapshot
            // solution: after moving request into service, we can use localProfileConfig
            auto it = m_lastSeenStatus.find(screenName);
            if (it == m_lastSeenStatus.end()) {
                m_lastSeenStatus[screenName] = ScreenStatus::kDisconnected;
            }

            if (m_lastSeenStatus[screenName] != status) {
                Screen screen = m_localProfileConfig->getScreen(screenName);
                screen.status(status);
                m_cloudClient->fakeScreenStatusUpdate(screen);
            }

            m_lastSeenStatus[screenName] = status;
        }
    );

    m_process->output.connect (
        [this](std::string const& line) {
            if (line.find("disconnected from server") != std::string::npos) {
                this->restart();
            }
        },
    boost::signals2::at_front);

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
                                                          std::vector<Screen> const& removed) {

        auto removedLocal = std::find_if (begin(removed), end(removed), [this](auto const& screen) {
            return (screen.id() == m_userConfig->screenId());
        });

        if (removedLocal != end(removed)) {
            serviceLog()->debug ("Local screen removed from profile");
            m_cloudClient->shutdownWebsocket();
            m_userConfig->reset();
            m_userConfig->save();
            m_rpc.server()->publish ("synergy.auth.logout");
            m_process->shutdown();
            return;
        }

        m_ioService.post([this]() {
            auto processMode = m_process->processMode();
            if (processMode == ProcessMode::kServer) {
                serviceLog()->debug ("restarting core because local profile screen "
                                     "set changed, mode={}",
                                     processModeToString(processMode));
                m_process->startServer();

                // HACK: send server claim in local network
                // reason: when there is a new screen added or removed, server needs to restart itself
                // on the client side, it relis on connection timeout to reconnect to the new server
                // sending this local claim will trigger clients to reconnect immediately
                notifyServerClaim(m_userConfig->screenId());
            }
        });
    });

    m_localProfileConfig->screenStatusChanged.connect([this](int64_t screenId){
        m_ioService.post([this, screenId]() {
            // HACK: update status record
            // reason: this status record is used for adjusting the frequency of updating
            // status after we moving update request into service
            Screen screen = m_localProfileConfig->getScreen(screenId);
            m_lastSeenStatus[screen.name()] = screen.status();
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

    m_errorMonitor->monitor(*m_process);
}

CoreManager::~CoreManager () noexcept {
    shutdown();
}

void
CoreManager::shutdown() {
    m_process->shutdown();
}

bool
CoreManager::restart()
{
    if (m_process->processMode() == ProcessMode::kUnknown) {
        /* Don't restart when we don't have a mode */
        return false;
    }

    m_process->start (m_processCommand->generate
                      (m_process->processMode() == ProcessMode::kServer));
    return true;
}

bool
CoreManager::setRunAsUid(std::string runAsUid)
{
    if (m_processCommand->setRunAsUid (std::move (runAsUid))) {
        return restart();
    }
    return false;
}

bool
CoreManager::setDisplay(std::string display)
{
    if (m_processCommand->setDisplay (std::move (display))) {
        return restart();
    }
    return false;
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

CoreErrorMonitor&
CoreManager::errorMonitor() const
{
    return *m_errorMonitor;
}

CoreStatusMonitor&
CoreManager::statusMonitor() const
{
    return m_process->statusMonitor();
}
