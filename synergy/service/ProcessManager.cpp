#include <synergy/service/ProcessManager.h>
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

namespace bp = boost::process;
namespace bs = boost::system;
using boost::optional;

template <typename T, typename... Args> static inline
std::unique_ptr<T>
make_unique (Args&&... args) {
    return std::unique_ptr<T>(new T (std::forward<Args>(args)...));
}

enum class ClientState: int {
    Disconnected = 0,
    Connecting   = 1,
    Connected    = 2
};

class ProcessManagerImpl {
public:
    ProcessManagerImpl (boost::asio::io_service& io,
                        std::vector<std::string> command)
        : m_command(std::move(command)), m_ioStrand (io),
          m_outPipe (io), m_errorPipe (io) {
    }

    void start (ProcessManager& manager);
    void shutdown ();

public:
    std::map<std::string, ClientState> m_clients;
    std::vector<boost::signals2::connection> m_signals;
    std::vector<std::string> m_command;
    bool m_expectingExit = false;

    boost::asio::io_service::strand m_ioStrand;
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
}

void
ProcessManagerImpl::shutdown() {
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

ProcessManager::ProcessManager (boost::asio::io_service &io)
    : m_ioService (io) {
}

ProcessManager::~ProcessManager () noexcept {
    shutdown();
}

void
ProcessManager::start (std::vector<std::string> command) {
    if (m_impl) {
        m_impl->m_expectingExit = true;

        onExit.connect_extended ([this, &process = m_impl->m_process]
                                 (auto& connection) {
            connection.disconnect();
            process->join();
            m_impl->m_expectingExit = false;
        });

        shutdown();
    }

    auto binary = boost::filesystem::path (command[0]).stem();
    auto nameArg = std::find (begin(command), end(command), "--name");

    if ((nameArg == end(command)) || (++nameArg == end(command))) {
        throw;
    }

    assert (!m_impl);
    m_impl = std::make_unique<ProcessManagerImpl>(m_ioService,
                                                  std::move (command));
    auto& signals = m_impl->m_signals;
    auto& clients = m_impl->m_clients;
    auto localScreenName = *nameArg;
    auto& localState = clients[localScreenName];
    using boost::algorithm::contains;

    if (binary.string() == "synergyc") {
        localState = ClientState::Disconnected;

        signals.emplace_back (
            onOutput.connect ([this, &localState](std::string const& line) {
                if (contains (line, "connected to server")) {
                    assert (localState == ClientState::Connecting);
                    localState = ClientState::Connected;
                    onReady();
                }
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            onOutput.connect ([this, &localState](std::string const& line) {
                if (contains (line, "disconnected from server")) {
                    assert (localState != ClientState::Disconnected);
                    localState = ClientState::Disconnected;
                }
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            onOutput.connect ([this, &localState](std::string const& line) {
                if (contains (line, "connecting to")) {
                    assert (localState == ClientState::Disconnected);
                    localState = ClientState::Connecting;
                }
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            onOutput.connect ([this](std::string const& line) {
                if (contains (line, "local input detected")) {
                }
            }, boost::signals2::at_front)
        );
    }
    else {
        localState = ClientState::Connecting;

        signals.emplace_back (
            onOutput.connect_extended ([this, &localState]
                                       (auto& connection, std::string const& line) {
                if (contains (line, "started server, waiting for clients")) {
                    connection.disconnect();
                    assert (localState == ClientState::Connecting);
                    localState = ClientState::Connected;
                    onReady();
                }
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            onOutput.connect ([this, &clients](std::string const& line) {
                static const boost::regex rgx("client \"(.*)\" has connected$");
                boost::match_results<std::string::const_iterator> results;
                if (regex_search (line, results, rgx)) {
                    assert (results.size() == 2);
                    auto screenName = results[1].str();
                    auto& status = clients[screenName];
                    assert (status == ClientState::Disconnected);
                    status = ClientState::Connected;
                }
            }, boost::signals2::at_front)
        );

        signals.emplace_back (
            onOutput.connect ([this, &clients](std::string const& line) {
                static const boost::regex rgx("client \"(.*)\" has disconnected$");
                boost::match_results<std::string::const_iterator> results;
                if (regex_search (line, results, rgx)) {
                    assert (results.size() == 2);
                    auto screenName = results[1].str();
                    auto& status = clients[screenName];
                    assert (status == ClientState::Connected);
                    status = ClientState::Disconnected;
                }
            }, boost::signals2::at_front)
        );
    }

    m_impl->start (*this);
}

void
ProcessManager::shutdown() {
    if (m_impl) {
        m_impl->shutdown();
        m_impl.reset();
    }
}
