#include <synergy/service/ProcessManager.h>

#include <synergy/common/DirectoryManager.h>
#include <synergy/common/ConfigGen.h>
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

namespace bp = boost::process;
namespace bs = boost::system;
using boost::optional;

static const int kConnectingTimeout = 3;

template <typename T, typename... Args> static inline
std::unique_ptr<T> make_unique (Args&&... args) {
    return std::unique_ptr<T>(new T (std::forward<Args>(args)...));
}

class ProcessManagerImpl {
public:
    ProcessManagerImpl (boost::asio::io_service& io,
                        std::vector<std::string> command)
        : m_command(std::move(command)), m_ioStrand (io), m_connectionTimer(io),
          m_outPipe (io), m_errorPipe (io) {
    }

    void start (ProcessManager& manager);
    void shutdown ();
    void onScreenStatusChanged (std::string const& screenName,
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
    manager.onOutput (line);

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
ProcessManagerImpl::start (ProcessManager& manager) {
    assert (!m_command.empty());
    assert (!m_process);

    m_process.emplace (
        m_command,
        bp::std_in.close (),
        bp::std_out > m_outPipe,
        bp::std_err > m_errorPipe,
        bp::on_exit = [this, &manager](int exit, std::error_code const& ec){
            if (m_expectingExit) {
                manager.onExit();
            } else {
                manager.onUnexpectedExit();
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
                return asyncReadLines (manager, m_ioStrand, m_outPipe,
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
                return asyncReadLines (manager, m_ioStrand, m_errorPipe,
                                       m_errorBuffer, m_lineBuffer, ec, bytes);
        })
    );

    mainLog()->debug("core process started");
}

void
ProcessManagerImpl::shutdown() {
    auto& ioService = m_ioStrand.get_io_service();

    mainLog()->debug("stopping core process");

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

    mainLog()->debug("core process stopped");
}

ProcessManager::ProcessManager (boost::asio::io_service& io, std::shared_ptr<Profile> localProfile) :
    m_ioService (io),
    m_localProfile(localProfile)
{
    m_localProfile->serverChanged.connect([this](int64_t serverId){
        // TODO: check if we need to reconnect to the new server or switch to server mode, generate config file and start synergys
    });

    m_localProfile->screenPositionChanged.connect([this](int64_t){
        // TODO: if we are the server, regenerate a config file and restart synergys
    });

    m_localProfile->screenSetChanged.connect([this](std::vector<Screen>, std::vector<Screen>){
        // TODO: if we are the server, regenerate a config file and restart synergys
    });
}

ProcessManager::~ProcessManager () noexcept {
    shutdown();
}

static std::string
getCommandBinaryName (std::vector<std::string> const& command) {
    return boost::filesystem::path (command[0]).stem().string();
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
            //screenConnectionError (screenName, kConnectionTimeout);
        });
    } else {
        timer.cancel();
    }
}

void
ProcessManager::start (std::vector<std::string> command) {
    mainLog()->debug("starting core process");

    if (m_impl) {
        m_impl->m_expectingExit = true;
        onExit.connect_extended ([this, &process = m_impl->m_process]
                                 (auto& connection) {
            connection.disconnect();

            if (process) {
                process->join();
            }
            else {
                mainLog()->error("can't join process, not initialized");
            }

            m_impl->m_expectingExit = false;
        });
        shutdown();
        assert (!m_impl);
    }

    auto const binary = getCommandBinaryName (command);
    auto const localScreenName = getCommandLocalScreenName (command);

    m_impl = std::make_unique<ProcessManagerImpl>
                (m_ioService, std::move (command));
    auto& localState = m_impl->m_clients[localScreenName];
    using boost::algorithm::contains;

    if (binary == "synergyc") {
        localState = ScreenStatus::kDisconnected;
        auto& signals = m_impl->m_signals;

        signals.emplace_back (
            onOutput.connect ([this, &localState,
                              localScreenName](std::string const& line) {
                if (!contains (line, "connected to server")) {
                    return;
                }
                assert (localState == ScreenStatus::kConnecting);
                localState = ScreenStatus::kConnected;
                //screenStatusChanged (localScreenName, localState);
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            onOutput.connect ([this, &localState,
                              localScreenName](std::string const& line) {
                if (!contains (line, "disconnected from server")) {
                    return;
                }
                assert (localState != ScreenStatus::kDisconnected);
                localState = ScreenStatus::kDisconnected;
                //screenStatusChanged (localScreenName, localState);
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            onOutput.connect ([this, &localState,
                              localScreenName](std::string const& line) {
                if (!contains (line, "connecting to")) {
                    return;
                }
                localState = ScreenStatus::kConnecting;
                //screenStatusChanged (localScreenName, localState);
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            onOutput.connect ([this](std::string const& line) {
                if (contains (line, "local input detected")) {
                    localInputDetected();
                }
            }, boost::signals2::at_front)
        );
    } else {
        localState = ScreenStatus::kConnecting;
        auto& signals = m_impl->m_signals;

        signals.emplace_back (
            onOutput.connect_extended ([this, &localState, localScreenName]
                                (auto& connection, std::string const& line) {
                if (!contains (line, "started server, waiting for clients")) {
                    return;
                }
                connection.disconnect();
                assert (localState == ScreenStatus::kConnecting);
                localState = ScreenStatus::kConnected;
                //screenStatusChanged (localScreenName, localState);
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            onOutput.connect ([this](std::string const& line) {
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
                //screenStatusChanged (std::move (screenName), status);
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            onOutput.connect ([this](std::string const& line) {
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
                //screenStatusChanged (std::move (screenName), status);
            }, boost::signals2::at_front)
        );
    }

    //screenStatusChanged (localScreenName, localState);
    m_impl->start (*this);
}

void
ProcessManager::shutdown() {
    if (m_impl) {
        m_impl->shutdown();
        m_impl.reset();
    }
}
