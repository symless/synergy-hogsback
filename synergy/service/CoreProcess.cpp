#include <synergy/service/CoreProcess.h>

#include <synergy/common/Screen.h>
#include <synergy/common/ConfigGen.h>
#include <synergy/common/DirectoryManager.h>
#include <synergy/common/ProcessCommand.h>
#include <synergy/common/ConfigGen.h>
#include <synergy/common/UserConfig.h>
#include <synergy/common/NetworkParameters.h>
#include <synergy/common/Hostname.h>
#include <synergy/service/CoreStatusMonitor.h>
#include <synergy/service/ServiceLogs.h>
#include <synergy/service/CoreProcessImpl.h>
#include <synergy/service/router/protocol/v2/MessageTypes.hpp>

#include <cassert>
#include <vector>
#include <string>
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
    m_statusMonitor(std::make_unique<CoreStatusMonitor>(m_userConfig, m_localProfileConfig)),
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

    output.connect (
        [this](std::string const& line) {
            if (line.find("local input detected") != std::string::npos) {
                localInputDetected();
            }
        });

    output.connect (
        [this] (std::string const& line) {
            if (line.find("started server, waiting for clients") != std::string::npos) {
                serverReady();
            }
        });
}

CoreProcess::~CoreProcess () noexcept {
    shutdown();
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
        start(m_processCommand->serverCmd(localScreenName()));
    } catch (const std::exception& ex) {
        serviceLog()->error ("failed to start server core process: {}", ex.what());
        m_impl.reset();
        assert (!m_impl);
        throw;
    }
}

void
CoreProcess::startClient(int const serverId)
{
    serviceLog()->debug("starting core client process");

    m_processMode = ProcessMode::kClient;
    m_currentServerId = serverId;

    try {
        start (m_processCommand->clientCmd(localScreenName()));
    } catch (const std::exception& ex) {
        serviceLog()->error("failed to start client core process: {}", ex.what());
        m_impl.reset();
        assert (!m_impl);
        throw;
    }
}

void
CoreProcess::start (std::vector<std::string> command)
{
    if (m_disabled) {
        serviceLog()->debug("core process is disabled");
        return;
    }

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

    m_impl = std::make_unique<CoreProcessImpl>(*this, m_ioService,
                                               std::move (command));

     m_statusMonitor->update(m_userConfig->screenId(), ScreenStatus::kConnecting);
     m_statusMonitor->monitor(*this);

    m_impl->start();
}

void
CoreProcess::writeConfigurationFile()
{
    auto configPath = DirectoryManager::instance()->profileDir() / kCoreConfigFile;
    createConfigFile(configPath.string(), m_localProfileConfig->screens());
}

bool CoreProcess::disabled() const
{
    return m_disabled;
}

std::string
CoreProcess::localScreenName()
{
    auto& s = m_localProfileConfig->getScreen(
                                        m_userConfig->screenId());
    return s.name();
}

CoreStatusMonitor&
CoreProcess::statusMonitor() const
{
    return *m_statusMonitor;
}

void
CoreProcess::setDisabled(bool disabled)
{
    m_disabled = disabled;
}
