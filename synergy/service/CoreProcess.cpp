#include <synergy/service/CoreProcess.h>

#include <synergy/common/Screen.h>
#include <synergy/common/ConfigGen.h>
#include <synergy/common/DirectoryManager.h>
#include <synergy/common/ProcessCommand.h>
#include <synergy/common/ConfigGen.h>
#include <synergy/common/UserConfig.h>
#include <synergy/common/NetworkParameters.h>
#include <synergy/service/ServiceLogs.h>
#include <synergy/service/CoreProcessImpl.h>
#include <synergy/service/router/protocol/v2/MessageTypes.hpp>
#include <synergy/service/router/Router.hpp>

#include <cassert>
#include <vector>
#include <string>
#include <boost/regex.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/join.hpp>

auto const kUnexpectedExitRetryTime = std::chrono::seconds (2);

class ClaimMessageHandler final {
public:
    explicit ClaimMessageHandler (CoreProcess& coreProcess) :
        m_coreProcess(coreProcess)
    {
    }

    void operator() (Message const&, std::uint32_t source) const;
    void handle (ServerClaim const&, std::uint32_t source) const;

    template <typename T>
    void handle (T const&,  std::uint32_t) const;

private:
    CoreProcess& m_coreProcess;
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
    if (msg.profile_id != m_coreProcess.m_userConfig->profileId()) {
        serviceLog()->debug ("ignore a server claim message from a different profile, current profile:{}, calim message in profile:{}",
                             m_coreProcess.m_userConfig->profileId(), msg.profile_id);
        return;
    }

    serviceLog()->debug("handling router message: server claim, mode={} thisId={} serverId={} lastServerId={}",
        processModeToString(m_coreProcess.m_processMode), m_coreProcess.m_userConfig->screenId(), msg.screen_id, m_coreProcess.m_currentServerId);

    m_coreProcess.onServerChanged(msg.screen_id);
}

template <typename T> inline
void
ClaimMessageHandler::handle (T const&, std::uint32_t) const {
}

CoreProcess::CoreProcess (boost::asio::io_service& io,
                          std::shared_ptr<UserConfig> userConfig,
                          std::shared_ptr<ProfileConfig> localProfileConfig,
                          Router& router):
    m_ioService (io),
    m_userConfig (std::move (userConfig)),
    m_localProfileConfig (std::move (localProfileConfig)),
    m_retryTimer (io),
    m_processMode(ProcessMode::kUnknown),
    m_currentServerId(-1),
    m_router(router)
{
    m_messageHandler = std::make_unique<ClaimMessageHandler> (*this);
    m_router.on_receive.connect (*m_messageHandler);

    m_localProfileConfig->profileServerChanged.connect([this](int64_t const serverId) {
        m_ioService.post([this, serverId] () {
            serviceLog()->debug("handling cloud message: server claim, mode={} thisId={} serverId={} lastServerId={}",
                processModeToString(m_processMode), m_userConfig->screenId(), serverId, m_currentServerId);

            onServerChanged(serverId);
        });
    });

    m_localProfileConfig->screenPositionChanged.connect([this](int64_t const /* targetScreenID */){
        m_ioService.post([this]() {
            serviceLog()->debug ("restarting core because local profile screen "
                                 "position changed, mode={}",
                                 processModeToString(m_processMode));
            if (m_processMode == ProcessMode::kServer) {  
                startServer();

                // HACK: send server claim in local network
                // reason: when there is a screen position change, server needs to restart itself
                // on the client side, it relis on connection timeout to reconnect to the new server
                // sending this local claim will trigger clients to reconnect immediately
                broadcastServerClaim(m_userConfig->screenId());
            }
        });
    });

    m_localProfileConfig->screenSetChanged.connect([this](std::vector<Screen> const&,
                                                          std::vector<Screen> const&) {
        m_ioService.post([this]() {
            serviceLog()->debug ("restarting core because local profile screen "
                                 "set changed, mode={}",
                                 processModeToString(m_processMode));
            if (m_processMode == ProcessMode::kServer) {
                startServer();

                // HACK: send server claim in local network
                // reason: when there is a new screen added or removed, server needs to restart itself
                // on the client side, it relis on connection timeout to reconnect to the new server
                // sending this local claim will trigger clients to reconnect immediately
                broadcastServerClaim(m_userConfig->screenId());
            }
        });
    });

    expectedExit.connect ([this]() {
        m_impl.reset();
        if (!m_nextCommand.empty()) {
            this->start (std::move (m_nextCommand));
        }
    });

    unexpectedExit.connect ([this]() {
        m_impl.reset();
        if (!m_lastCommand.empty()) {
            boost::system::error_code ec;
            m_retryTimer.expires_from_now (kUnexpectedExitRetryTime);
            m_retryTimer.async_wait ([&, command = std::move(m_lastCommand)]
                                     (auto const ec) {
                if (ec == boost::asio::error::operation_aborted) {
                    return;
                } else if (ec) {
                    throw boost::system::system_error (ec, ec.message());
                }
                this->start (std::move (command));
            });
        }
    });

    screenStatusChanged.connect (
        [this](std::string const& screenName, ScreenStatus const status) {
            if (m_impl) {
                m_impl->onScreenStatusChanged (screenName, status);
            }
        }
    );
}

CoreProcess::~CoreProcess () noexcept {
    shutdown();
}

static std::string
getCommandLocalScreenName (std::vector<std::string> const& command) {
    auto nameArg = std::find (begin(command), end(command), "--name");
    if ((nameArg == end(command)) || (++nameArg == end(command))) {
        throw std::runtime_error("Unable to determine local screen name from "
                                 "core process command line.");
    }
    return *nameArg;
}

static std::string
getCommandProcessMode (std::vector<std::string> const& command) {
    auto const clientArg = std::find (begin(command), end(command), "--client");
    if (clientArg != end(command)) {
        return *clientArg;
    }
    auto const serverArg = std::find (begin(command), end(command), "--server");
    if (serverArg != end(command)) {
        return *serverArg;
    }
    throw std::runtime_error("Unable to determine core process mode from core "
                             "process command line.");
}

void
CoreProcess::start (std::vector<std::string> command)
{
    using boost::algorithm::contains;
    boost::system::error_code ec;
    m_retryTimer.cancel (ec);

    if (m_impl) {
        serviceLog()->debug("core process already running, attempting to stop");
        m_nextCommand = std::move (command);
        shutdown();
        return;
    }

    serviceLog()->debug("starting core process with command: {}",
                        boost::algorithm::join(command, " "));

    m_lastCommand = command;
    auto const localScreenName = getCommandLocalScreenName (command);
    auto const processMode = getCommandProcessMode (command);

    m_impl = std::make_unique<CoreProcessImpl>(*this, m_ioService,
                                               std::move (command));
    auto& localScreenState = m_impl->m_screenStates[localScreenName];
    auto& signals = m_impl->m_signals;

    if (processMode == "--client") {
        localScreenState = ScreenStatus::kDisconnected;

        signals.emplace_back (
            output.connect ([this, &localScreenState, localScreenName](std::string const& line) {
                if (!contains (line, "connected to server")) {
                    return;
                }
                localScreenState = ScreenStatus::kConnected;
                screenStatusChanged(localScreenName, localScreenState);
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            output.connect ([this, &localScreenState, localScreenName](std::string const& line) {
                if (!contains (line, "disconnected from server")) {
                    return;
                }
                localScreenState = ScreenStatus::kDisconnected;
                screenStatusChanged(localScreenName, localScreenState);
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            output.connect ([this, &localScreenState, localScreenName](std::string const& line) {
                if (!contains (line, "connecting to")) {
                    return;
                }
                localScreenState = ScreenStatus::kConnecting;
                screenStatusChanged(localScreenName, localScreenState);
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            output.connect ([this](std::string const& line) {
                if (contains (line, "local input detected")) {
                    localInputDetected();
                }
            }, boost::signals2::at_front)
        );
    }
    else if (processMode == "--server") {
        localScreenState = ScreenStatus::kConnecting;

        signals.emplace_back (
            output.connect_extended ( [this, &localScreenState, localScreenName]
                                      (auto& connection, std::string const& line) {
                if (!contains (line, "started server, waiting for clients")) {
                    return;
                }
                connection.disconnect();
                localScreenState = ScreenStatus::kConnected;
                screenStatusChanged(localScreenName, localScreenState);
                serverReady();
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            output.connect ([this](std::string const& line) {
                static boost::regex const rgx("client \"(.*)\" has connected$");
                boost::match_results<std::string::const_iterator> results;
                if (!regex_search (line, results, rgx)) {
                    return;
                }
                auto clientScreenName = results[1].str();
                auto& clientScreenStatus = m_impl->m_screenStates[clientScreenName];
                clientScreenStatus = ScreenStatus::kConnected;
                screenStatusChanged(std::move (clientScreenName), clientScreenStatus);
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            output.connect ([this](std::string const& line) {
                static boost::regex const rgx ("client \"(.*)\" has disconnected$");
                boost::match_results<std::string::const_iterator> results;
                if (!regex_search (line, results, rgx)) {
                    return;
                }
                auto clientScreenName = results[1].str();
                auto& clientScreenStatus = m_impl->m_screenStates[clientScreenName];
                clientScreenStatus = ScreenStatus::kDisconnected;
                screenStatusChanged(std::move (clientScreenName), clientScreenStatus);
            }, boost::signals2::at_front)
        );
    }

    screenStatusChanged(localScreenName, localScreenState);
    m_impl->start();
}

void
CoreProcess::shutdown() {
    if (!m_impl) {
        serviceLog()->debug("ignoring core process shutdown request, core is "
                            "not running");
        return;
    }
    m_impl->shutdown();
}

void
CoreProcess::writeConfigurationFile()
{
    auto configPath = DirectoryManager::instance()->profileDir() / kCoreConfigFile;
    createConfigFile(configPath.string(), m_localProfileConfig->screens());
}

void
CoreProcess::startServer()
{
    serviceLog()->debug("starting core server process");

    m_processMode = ProcessMode::kServer;
    m_currentServerId = m_userConfig->screenId();
    writeConfigurationFile();

    ProcessCommand command;
    command.setLocalHostname(boost::asio::ip::host_name());
    command.setRunAsUid(m_runAsUid);

    try {
        start(command.generate(true));
    } catch (const std::exception& ex) {
        serviceLog()->error ("failed to start server core process: {}", ex.what());
        m_impl.reset();
        assert (!m_impl);
    }
}

void
CoreProcess::startClient(int const serverId)
{
    serviceLog()->debug("starting core client process");

    m_processMode = ProcessMode::kClient;
    m_currentServerId = serverId;

    ProcessCommand command;
    command.setLocalHostname(boost::asio::ip::host_name());
    command.setRunAsUid(m_runAsUid);

    try {
        start (command.generate(false));
    } catch (const std::exception& ex) {
        serviceLog()->error("failed to start client core process: {}", ex.what());
        m_impl.reset();
        assert (!m_impl);
    }
}

ProcessMode
CoreProcess::processMode() const
{
    return m_processMode;
}

void
CoreProcess::switchServer(int64_t serverId)
{
    onServerChanged(serverId);
}

void
CoreProcess::broadcastServerClaim(int64_t serverId)
{
    ServerClaim serverClaimMessage;
    serverClaimMessage.profile_id = m_userConfig->profileId();
    serverClaimMessage.screen_id = serverId;
    Message message(serverClaimMessage);
    message.setTtl(m_localProfileConfig->screens().size());

    m_router.broadcast(message);
}

int
CoreProcess::currentServerId() const
{
    return m_currentServerId;
}

void
CoreProcess::setRunAsUid(std::string runAsUid)
{
    m_runAsUid = std::move (runAsUid);
}

void
CoreProcess::onServerChanged(int64_t const serverId)
{
    switch (m_processMode) {
        case ProcessMode::kServer: {
            // when server changes from local screen to another screen
            if (m_userConfig->screenId() != serverId) {
               startClient(serverId);
            }
            else {
                serviceLog()->debug("core is already in server mode, ingore switching");
            }

            break;
        }
        case ProcessMode::kClient: {
            // when local screen becomes the server
            if (m_userConfig->screenId() == serverId) {
                startServer();
                break;
            }

            // when another screen, not local screen, claims to be the server
            startClient(serverId);

            break;
        }
        case ProcessMode::kUnknown: {
            if (m_userConfig->screenId() == serverId) {
                startServer();
            }
            else {
                startClient(serverId);
            }

            break;
        }
    }
}
