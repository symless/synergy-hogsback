#include <synergy/service/ProcessManager.h>

#include <synergy/service/ConnectivityTester.h>
#include <synergy/common/ConfigGen.h>
#include <synergy/common/DirectoryManager.h>
#include <synergy/common/ProcessCommand.h>
#include <synergy/common/ConfigGen.h>
#include <synergy/common/UserConfig.h>
#include <synergy/service/Logs.h>
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

template <typename T, typename... Args> static inline
std::unique_ptr<T> make_unique (Args&&... args) {
    return std::unique_ptr<T>(new T (std::forward<Args>(args)...));
}

class ProcessManagerImpl {
public:
    ProcessManagerImpl (
        ProcessManager& parent,
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
    ProcessManager& m_parent;
};

template <typename Strand, typename Pipe, typename Buffer, typename Line>
static void
asyncReadLines (ProcessManager& manager, Strand& strand, Pipe& pipe,
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
ProcessManagerImpl::start () {
    assert (!m_command.empty());
    assert (!m_process);

    m_process.emplace (
        m_command,
        bp::std_in.close (),
        bp::std_out > m_outPipe,
        bp::std_err > m_errorPipe,
        bp::on_exit = [this](int exit, std::error_code const& ec){
            mainLog()->debug("core process exited: code={} expected={}", exit, m_expectingExit);
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

    mainLog()->debug("core process started, id={}", m_process->id());
}

void
ProcessManagerImpl::shutdown()
{
    mainLog()->debug("stopping core process");

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

ProcessManager::ProcessManager (boost::asio::io_service& io, std::shared_ptr<UserConfig> userConfig, std::shared_ptr<ProfileConfig> localProfileConfig) :
    m_ioService (io),
    m_userConfig(userConfig),
    m_localProfileConfig(localProfileConfig),
    m_connectivityTester (std::make_unique<ConnectivityTester>(m_ioService, m_localProfileConfig)),
    m_proccessMode(ProcessMode::kUnknown),
    m_lastServerId(-1)
{
    m_localProfileConfig->profileServerChanged.connect([this](int64_t serverId) {
        m_ioService.post([this, serverId] () {

            mainLog()->debug("handling local profile server changed, mode={} thisId={} serverId={} lastServerId={}",
                processModeToString(m_proccessMode), m_userConfig->screenId(), serverId, m_lastServerId);

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
                    writeConfigurationFile();
                    startServer();
                }
                // when another screen, not local screen, claims to be the server
                else if (m_lastServerId != serverId) {
                    startClient(serverId);
                }

                break;
            }
            case ProcessMode::kUnknown: {
                if (m_userConfig->screenId() == serverId) {
                    writeConfigurationFile();
                    startServer();
                }
                else {
                    startClient(serverId);
                }

                break;
            }
            }
        });
    });

    m_localProfileConfig->screenPositionChanged.connect([this](int64_t){
        m_ioService.post([this] () {

            mainLog()->debug("handling local profile screen position changed, mode={}",
                processModeToString(m_proccessMode));

            if (m_proccessMode == ProcessMode::kServer) {
                startServer();
            }
        });
    });

    m_localProfileConfig->screenSetChanged.connect([this](std::vector<Screen>, std::vector<Screen>){
        m_ioService.post([this] () {

            mainLog()->debug("handling local profile screen set changed, mode={}",
                processModeToString(m_proccessMode));

            if (m_proccessMode == ProcessMode::kServer) {
                startServer();
            }
        });
    });

    m_connectivityTester->newReportGenerated.connect([this](int screenId, std::string successfulIp, std::string) {

        mainLog()->debug("handling new report from connectivity tester, screenId={} successfulIp={} lastServerId={}",
            screenId, successfulIp, m_lastServerId);

        if (m_lastServerId == screenId &&
            !successfulIp.empty()) {
            startClient(m_lastServerId);
        }
    });
}

ProcessManager::~ProcessManager () noexcept {
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
ProcessManagerImpl::onScreenStatusChanged
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
ProcessManager::start (std::vector<std::string> command)
{
    if (m_impl) {
        mainLog()->debug("core process already running, attempting to stop");
        m_nextCommand = command;
        shutdown();
        return;
    }

    mainLog()->debug("starting core process with command: {}", ba::join(command, " "));

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

    m_impl = std::make_unique<ProcessManagerImpl>(*this, m_ioService, std::move (command));
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

    screenStatusChanged(localScreenName, localState);
    m_impl->start();
}

void
ProcessManager::shutdown() {

    if (!m_impl) {
        mainLog()->debug("core process is not running, ignoring shutdown");
        return;
    }

    // control which exited event is invoked
    m_impl->m_expectingExit = true;

    expectedExit.connect_extended([this](auto& connection) {
        connection.disconnect();

        if (m_impl->m_process) {
            m_impl->m_process->join();
        }
        else {
            mainLog()->error("can't join process, not initialized");
        }

        // TODO: figure out how to stop qt freaking out when this is called
        m_impl.reset();

        mainLog()->debug("core process shutdown complete");

        if (!m_nextCommand.empty()) {
            assert(!m_impl);
            this->start(std::move(m_nextCommand));
        }
    });

    m_impl->shutdown();
}

void ProcessManager::writeConfigurationFile()
{
    auto configPath = DirectoryManager::instance()->profileDir() / kCoreConfigFile;
    createConfigFile(configPath.string(), m_localProfileConfig->screens());
}

void ProcessManager::startServer()
{
    mainLog()->debug("starting core server process");

    m_proccessMode = ProcessMode::kServer;
    m_lastServerId = m_userConfig->screenId();

    writeConfigurationFile();

    ProcessCommand command;
    command.setLocalHostname(boost::asio::ip::host_name());

    try {
        start(command.generate(true));
    }
    catch (const std::exception& ex) {
        mainLog()->error("failed to start server core process: {}", ex.what());
        m_impl.reset();
        assert(!m_impl);
    }
}

void ProcessManager::startClient(int serverId)
{
    mainLog()->debug("starting core client process");

    m_proccessMode = ProcessMode::kClient;
    m_lastServerId = serverId;

    ProcessCommand command;
    command.setLocalHostname(boost::asio::ip::host_name());

    auto screen = m_localProfileConfig->getScreen(serverId);
    std::vector<std::string> results = m_connectivityTester->getSuccessfulResults(serverId);
    if (results.empty()) {
        mainLog()->error("aborting client start, no successful connectivity test results");
        return;
    }

    // TODO: instead of using first successful result, test each and find the best
    // address to connect to, and have a `bestAddress` (cloud should make this decision)
    command.setServerAddress(results[0]);

    try {
        start(command.generate(false));
    }
    catch (const std::exception& ex) {
        mainLog()->error("failed to start client core process: {}", ex.what());
        m_impl.reset();
        assert(!m_impl);
    }
}
