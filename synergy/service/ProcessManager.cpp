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

namespace bp = boost::process;
namespace bs = boost::system;
using boost::optional;

class ProcessManagerImpl {
public:
    ProcessManagerImpl (boost::asio::io_service& io)
        : m_strand (io), m_outPipe (io), m_errorPipe (io) {
    }

    void start (ProcessManager& manager);
    void shutdown ();

    std::vector<std::string> m_command;
    bool m_awaitingExit = false;

    boost::asio::io_service::strand m_strand;
    boost::asio::streambuf m_outBuf;
    boost::asio::streambuf m_errorBuf;
    std::string m_lineBuf;
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
            if (m_awaitingExit) {
                manager.onExit();
            } else {
                manager.onUnexpectedExit();
            }
        },
        m_strand.get_io_service()
    );

    /* Start the stdout I/O loop */
    boost::asio::async_read_until (
        m_outPipe,
        m_outBuf,
        '\n',
        m_strand.wrap (
            [&](boost::system::error_code const& ec, std::size_t bytes) {
                return asyncReadLines (manager, m_strand, m_outPipe,
                                       m_outBuf, m_lineBuf, ec, bytes);
        })
    );

    /* Start the stderr I/O loop */
    boost::asio::async_read_until (
        m_errorPipe,
        m_errorBuf,
        '\n',
        m_strand.wrap (
            [&](boost::system::error_code const& ec, std::size_t bytes) {
                return asyncReadLines (manager, m_strand, m_errorPipe,
                                       m_errorBuf, m_lineBuf, ec, bytes);
        })
    );
}

void
ProcessManagerImpl::shutdown() {
    auto& ioService = m_strand.get_io_service();

    /* Cancel the standard stream I/O loops */
    m_outPipe.cancel();
    m_errorPipe.cancel();
    ioService.poll();

    m_process->terminate();
    ioService.poll();
}

template <typename T, typename... Args> static inline
std::unique_ptr<T>
make_unique (Args&&... args) {
    return std::unique_ptr<T>(new T (std::forward<Args>(args)...));
}

ProcessManager::ProcessManager (boost::asio::io_service &io)
    : m_ioService (io) {
}

ProcessManager::~ProcessManager () noexcept {
}

void
ProcessManager::start (std::vector<std::string> command) {
    if (m_impl) {
        m_impl->m_awaitingExit = true;

        onExit.connect_extended ([this, &process = m_impl->m_process]
                                 (auto& connection) {
            connection.disconnect();
            process->join();
            m_impl->m_awaitingExit = false;
        });

        shutdown();
    }

    assert (!m_impl);
    m_impl = std::make_unique<ProcessManagerImpl>(m_ioService);
    m_impl->m_command = std::move (command);
    m_impl->start (*this);
}

void
ProcessManager::shutdown() {
    if (m_impl) {
        m_impl->shutdown();
        m_impl.reset();
    }
}
