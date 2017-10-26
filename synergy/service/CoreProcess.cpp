#include <synergy/service/CoreProcess.h>

#include <synergy/common/ConfigGen.h>
#include <synergy/common/DirectoryManager.h>
#include <synergy/common/ProcessCommand.h>
#include <synergy/common/ConfigGen.h>
#include <synergy/common/UserConfig.h>
#include <synergy/service/ServiceLogs.h>
#include <synergy/common/NetworkParameters.h>

#include <boost/process.hpp>
#include <boost/process/async_pipe.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/optional.hpp>
#include <vector>
#include <string>
#include <boost/asio/strand.hpp>
#include <mutex>
#include <cassert>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/asio/steady_timer.hpp>
#include <synergy/common/Screen.h>
#include <boost/algorithm/string/join.hpp>

namespace bp = boost::process;
namespace bs = boost::system;
namespace ba = boost::algorithm;
using boost::optional;

static const int kConnectingTimeout = 3;

class CoreProcessImpl {
public:
    CoreProcessImpl (
        CoreProcess& parent,
        boost::asio::io_service& io,
        std::vector<std::string> command)
        :
        m_parent(parent),
        m_command(std::move(command)),
        m_ioStrand (io),
        m_connectionTimer(io),
        m_outPipe (io),
        m_errorPipe (io) {
    }

    ~CoreProcessImpl() {
        bs::error_code ec;
        m_outPipe.close(ec);
        m_errorPipe.close(ec);
    }

    void start();
    void shutdown();
    void onScreenStatusChanged(std::string const& screenName,
                                ScreenStatus status);

public:
    std::map<std::string, ScreenStatus> m_clients;
    std::vector<boost::signals2::connection> m_signals;
    std::vector<std::string> m_command;
    bool m_expectingExit = false;

    boost::asio::io_service::strand m_ioStrand;
    boost::asio::steady_timer m_connectionTimer;
    boost::asio::streambuf m_outputBuffer;
    boost::asio::streambuf m_errorBuffer;
    std::string m_lineBuffer;

    bp::async_pipe m_outPipe;
    bp::async_pipe m_errorPipe;
    optional<bp::child> m_process;

private:
    CoreProcess& m_parent;
};

template <typename Strand, typename Pipe, typename Buffer, typename Line>
static void
asyncReadLines (CoreProcess& manager, Strand& strand, Pipe& pipe,
                Buffer& buffer, Line& line, bs::error_code const& ec,
                std::size_t const bytes) {
    std::istream stream (&buffer);
    if (ec || !bytes || !std::getline (stream, line)) {
        return;
    }

    boost::algorithm::trim_right(line);
    manager.output (line);

    boost::asio::async_read_until (
        pipe,
        buffer,
        '\n',
        strand.wrap (
            [&](boost::system::error_code const& ec, std::size_t const bytes) {
                return asyncReadLines (manager, strand, pipe, buffer, line,
                                       ec, bytes);
        })
    );
}

void
CoreProcessImpl::start () {
    assert (!m_command.empty());
    assert (!m_process);

    m_process.emplace (
        m_command,
        bp::std_in.close (),
        bp::std_out > m_outPipe,
        bp::std_err > m_errorPipe,
        bp::on_exit = [this](int exit, std::error_code const& ec){
            serviceLog()->debug("core process exited: code={} expected={}", exit, m_expectingExit);
            if (m_expectingExit) {
                m_parent.expectedExit();
            } else {
                m_parent.unexpectedExit();
            }
        },
        m_ioStrand.get_io_service()
    );

    /* Start the stdout I/O loop */
    boost::asio::async_read_until (
        m_outPipe,
        m_outputBuffer,
        '\n',
        m_ioStrand.wrap (
            [&](boost::system::error_code const& ec, std::size_t bytes) {
                return asyncReadLines (m_parent, m_ioStrand, m_outPipe,
                                       m_outputBuffer, m_lineBuffer, ec, bytes);
        })
    );

    /* Start the stderr I/O loop */
    boost::asio::async_read_until (
        m_errorPipe,
        m_errorBuffer,
        '\n',
        m_ioStrand.wrap (
            [&](boost::system::error_code const& ec, std::size_t bytes) {
                return asyncReadLines (m_parent, m_ioStrand, m_errorPipe,
                                       m_errorBuffer, m_lineBuffer, ec, bytes);
        })
    );

    serviceLog()->debug("core process started, id={}", m_process->id());
}

void
CoreProcessImpl::shutdown()
{
    serviceLog()->debug("stopping core process");

    auto& ioService = m_ioStrand.get_io_service();

    /* Cancel the standard stream I/O loops */
    m_outPipe.cancel();
    m_errorPipe.cancel();
    ioService.poll();

    /* Disconnect internal signal handling */
    for (auto& signal: m_signals) {
        signal.disconnect();
    }

    m_process->terminate();
    ioService.poll();
}

std::string
processModeToString(ProcessMode mode) {
    switch (mode) {
        case ProcessMode::kServer:
            return "server";
        case ProcessMode::kClient:
            return "client";
        case ProcessMode::kUnknown:
            return "unknown";
    }
    return "";
}

CoreProcess::CoreProcess (boost::asio::io_service& io, std::shared_ptr<UserConfig> userConfig, std::shared_ptr<ProfileConfig> localProfileConfig) :
    m_ioService (io),
    m_userConfig(userConfig),
    m_localProfileConfig(localProfileConfig),
    m_proccessMode(ProcessMode::kUnknown),
    m_currentServerId(-1)
{
    m_localProfileConfig->profileServerChanged.connect([this](int64_t serverId) {
        m_ioService.post([this, serverId] () {
            onServerChanged(serverId);
        });
    });

    m_localProfileConfig->screenPositionChanged.connect([this](int64_t){
        m_ioService.post([this] () {

            serviceLog()->debug("handling local profile screen position changed, mode={}",
                processModeToString(m_proccessMode));

            if (m_proccessMode == ProcessMode::kServer) {
                startServer();
            }
        });
    });

    m_localProfileConfig->screenSetChanged.connect([this](std::vector<Screen>, std::vector<Screen>){
        m_ioService.post([this] () {

            serviceLog()->debug("handling local profile screen set changed, mode={}",
                processModeToString(m_proccessMode));

            if (m_proccessMode == ProcessMode::kServer) {
                startServer();
            }
        });
    });
}

CoreProcess::~CoreProcess () noexcept {
    shutdown();
}

static std::string
getCommandLocalScreenName (std::vector<std::string> const& command) {
    auto nameArg = std::find (begin(command), end(command), "--name");
    if ((nameArg == end(command)) || (++nameArg == end(command))) {
        throw;
    }
    return *nameArg;
}

void
CoreProcessImpl::onScreenStatusChanged
(std::string const& screenName, ScreenStatus const status) {
    auto& timer = m_connectionTimer;
    if (status == ScreenStatus::kConnecting) {
        if (timer.expires_at() > std::chrono::steady_clock::now()) {
            return;
        }
        timer.expires_from_now (std::chrono::seconds (kConnectingTimeout));
        timer.async_wait ([this, screenName](auto const& ec) {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            } else if (ec) {
                throw boost::system::system_error(ec);
            }
            m_parent.screenConnectionError(screenName);
        });
    } else {
        timer.cancel();
    }
}

void
CoreProcess::start (std::vector<std::string> command)
{
    if (m_impl) {
        serviceLog()->debug("core process already running, attempting to stop");
        m_nextCommand = command;
        shutdown();
        return;
    }

    m_lastCommand = command;
    serviceLog()->debug("starting core process with command: {}", ba::join(command, " "));

    auto const localScreenName = getCommandLocalScreenName (command);

    std::string mode;
    if (command.size() > 1) {
        mode = command[1];
        if ((mode != "--server") && (mode != "--client")) {
            throw std::runtime_error("Invalid core process mode: " + mode);
        }
    }

    if (mode.empty()) {
        throw std::runtime_error("Unable to determine core process mode.");
    }

    m_impl = std::make_unique<CoreProcessImpl>(*this, m_ioService, command);

    auto& localState = m_impl->m_clients[localScreenName];
    using boost::algorithm::contains;

    if (mode == "--client") {

        localState = ScreenStatus::kDisconnected;
        auto& signals = m_impl->m_signals;

        signals.emplace_back (
            output.connect ([this, &localState,
                              localScreenName](std::string const& line) {
                if (!contains (line, "connected to server")) {
                    return;
                }
                assert (localState == ScreenStatus::kConnecting);
                localState = ScreenStatus::kConnected;
                screenStatusChanged(localScreenName, localState);
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            output.connect ([this, &localState,
                              localScreenName](std::string const& line) {
                if (!contains (line, "disconnected from server")) {
                    return;
                }
                assert (localState != ScreenStatus::kDisconnected);
                localState = ScreenStatus::kDisconnected;
                screenStatusChanged(localScreenName, localState);
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            output.connect ([this, &localState,
                              localScreenName](std::string const& line) {
                if (!contains (line, "connecting to")) {
                    return;
                }
                localState = ScreenStatus::kConnecting;
                screenStatusChanged(localScreenName, localState);
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
    else if (mode == "--server") {

        localState = ScreenStatus::kConnecting;
        auto& signals = m_impl->m_signals;

        signals.emplace_back (
            output.connect_extended ([this, &localState, localScreenName]
                                (auto& connection, std::string const& line) {
                if (!contains (line, "started server, waiting for clients")) {
                    return;
                }
                connection.disconnect();
                assert (localState == ScreenStatus::kConnecting);
                localState = ScreenStatus::kConnected;
                screenStatusChanged(localScreenName, localState);
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            output.connect ([this](std::string const& line) {
                static boost::regex const rgx("client \"(.*)\" has connected$");
                boost::match_results<std::string::const_iterator> results;
                if (!regex_search (line, results, rgx)) {
                    return;
                }
                assert (results.size() == 2);
                auto screenName = results[1].str();
                auto& status = m_impl->m_clients[screenName];
                // assert (status == ScreenStatus::Disconnected);
                status = ScreenStatus::kConnected;
                screenStatusChanged(std::move (screenName), status);
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            output.connect ([this](std::string const& line) {
                static boost::regex const rgx ("client \"(.*)\" has disconnected$");
                boost::match_results<std::string::const_iterator> results;
                if (!regex_search (line, results, rgx)) {
                    return;
                }
                assert (results.size() == 2);
                auto screenName = results[1].str();
                auto& status = m_impl->m_clients[screenName];
                assert (status == ScreenStatus::kConnected);
                status = ScreenStatus::kDisconnected;
                screenStatusChanged(std::move (screenName), status);
            }, boost::signals2::at_front)
        );
    }
    else {
        throw std::runtime_error("Invalid core process mode: " + mode);
    }

    expectedExit.connect_extended([this](auto& connection) {
        connection.disconnect();

        this->handleShutdown();

        if (!m_nextCommand.empty()) {
            this->start(std::move(m_nextCommand));
        }
    });

    unexpectedExit.connect_extended([this](auto& connection) {
        connection.disconnect();

        this->handleShutdown();

        if (!m_lastCommand.empty()) {
            this->start(std::move(m_lastCommand));
        }
    });

    screenStatusChanged(localScreenName, localState);
    m_impl->start();
}

void
CoreProcess::shutdown() {

    if (!m_impl) {
        serviceLog()->debug("core process is not running, ignoring shutdown");
        return;
    }

    if (m_impl->m_expectingExit) {
        serviceLog()->debug("core process is shutting down, ignoring extra shutdown");
        return;
    }

    // control which exited event is invoked
    m_impl->m_expectingExit = true;

    m_impl->shutdown();
}

void CoreProcess::handleShutdown()
{
    if (m_impl->m_process) {
        try {
            m_impl->m_process->join();
        }
        catch (const std::exception& ex) {
            serviceLog()->error("can't join process: {}", ex.what());
        }
        catch (...) {
            serviceLog()->error("can't join process: unknown error");
        }
    }
    else {
        serviceLog()->error("can't join process, not initialized");
    }

    m_impl.reset();

    serviceLog()->debug("core process shutdown complete");
}

void CoreProcess::writeConfigurationFile()
{
    auto configPath = DirectoryManager::instance()->profileDir() / kCoreConfigFile;
    createConfigFile(configPath.string(), m_localProfileConfig->screens());
}

void CoreProcess::startServer()
{
    serviceLog()->debug("starting core server process");

    m_proccessMode = ProcessMode::kServer;
    m_currentServerId = m_userConfig->screenId();

    writeConfigurationFile();

    ProcessCommand command;
    command.setLocalHostname(boost::asio::ip::host_name());
    command.setRunAsUid(m_runAsUid);

    try {
        start(command.generate(true));
    }
    catch (const std::exception& ex) {
        serviceLog()->error("failed to start server core process: {}", ex.what());
        m_impl.reset();
        assert(!m_impl);
    }
}

void CoreProcess::startClient(int serverId)
{
    serviceLog()->debug("starting core client process");

    m_proccessMode = ProcessMode::kClient;
    m_currentServerId = serverId;

    ProcessCommand command;
    command.setLocalHostname(boost::asio::ip::host_name());
    command.setRunAsUid(m_runAsUid);

    command.setServerAddress(kServerProxyEndpoint);

    try {
        start(command.generate(false));
    }
    catch (const std::exception& ex) {
        serviceLog()->error("failed to start client core process: {}", ex.what());
        m_impl.reset();
        assert(!m_impl);
    }
}

ProcessMode CoreProcess::proccessMode() const
{
    return m_proccessMode;
}

int CoreProcess::currentServerId() const
{
    return m_currentServerId;
}

void
CoreProcess::setRunAsUid(const std::string& runAsUid)
{
    m_runAsUid = runAsUid;
}

void CoreProcess::onServerChanged(int64_t serverId)
{
    serviceLog()->debug("handling local profile server changed, mode={} thisId={} serverId={} lastServerId={}",
        processModeToString(m_proccessMode), m_userConfig->screenId(), serverId, m_currentServerId);

    switch (m_proccessMode) {
    case ProcessMode::kServer: {
        // when server changes from local screen to another screen
        if (m_userConfig->screenId() != serverId) {
           startClient(serverId);
        }

        break;
    }
    case ProcessMode::kClient: {
        // when local screen becomes the server
        if (m_userConfig->screenId() == serverId) {
            startServer();
        }
        // when another screen, not local screen, claims to be the server
        else if (m_currentServerId != serverId) {
            startClient(serverId);
        }

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
