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

#include <cassert>
#include <vector>
#include <string>
#include <boost/regex.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/join.hpp>

auto const kUnexpectedExitRetryTime = std::chrono::seconds (2);

CoreProcess::CoreProcess (boost::asio::io_service& io,
                          std::shared_ptr<UserConfig> userConfig,
                          std::shared_ptr<ProfileConfig> localProfileConfig,
                          std::shared_ptr<ProcessCommand> processCommand) :
    m_ioService (io),
    m_userConfig (std::move (userConfig)),
    m_localProfileConfig (std::move (localProfileConfig)),
    m_retryTimer (io),
    m_processMode(ProcessMode::kUnknown),
    m_currentServerId(-1),
    m_processCommand(processCommand),
    m_disabled(false)
{
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

int
CoreProcess::currentServerId() const
{
    return m_currentServerId;
}

ProcessMode
CoreProcess::processMode() const
{
    return m_processMode;
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
CoreProcess::startServer()
{
    serviceLog()->debug("starting core server process");

    m_processMode = ProcessMode::kServer;
    m_currentServerId = m_userConfig->screenId();
    writeConfigurationFile();

    try {
        start(m_processCommand->generate(true));
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

    try {
        start (m_processCommand->generate(false));
    } catch (const std::exception& ex) {
        serviceLog()->error("failed to start client core process: {}", ex.what());
        m_impl.reset();
        assert (!m_impl);
    }
}

void
CoreProcess::start (std::vector<std::string> command)
{
    if (m_disabled) {
        serviceLog()->debug("core process is disabled");
        return;
    }

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
    localScreenState = ScreenStatus::kConnecting;
    auto& signals = m_impl->m_signals;

    if (processMode == "--client") {
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

                startClient(currentServerId());
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
                if (!contains (line, "server is dead")) {
                    return;
                }

                Screen& screen = m_localProfileConfig->getScreen(m_currentServerId);
                screenStatusChanged(screen.name(), ScreenStatus::kDisconnected);
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

        signals.emplace_back (
            output.connect ([this](std::string const& line) {
                static boost::regex const rgx ("client \"(.*)\" is dead$");
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
CoreProcess::writeConfigurationFile()
{
    auto configPath = DirectoryManager::instance()->profileDir() / kCoreConfigFile;
    createConfigFile(configPath.string(), m_localProfileConfig->screens());
}

void CoreProcess::setDisabled(bool disabled)
{
    m_disabled = disabled;
}
