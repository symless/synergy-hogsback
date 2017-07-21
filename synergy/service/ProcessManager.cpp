#include <synergy/service/ProcessManager.h>
#include <boost/process.hpp>
#include <boost/process/async_pipe.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/optional.hpp>
#include <vector>
#include <string>
#include <boost/asio/strand.hpp>
#include <mutex>

namespace bp = boost::process;
namespace bs = boost::system;
using boost::optional;

class ProcessManagerImpl {
public:
    ProcessManagerImpl (asio::io_service& io)
        : m_ioStrand (io), m_outPipe (io), m_errorPipe (io) {
    }

    void start (ProcessManager& manager);

    std::mutex mtx;

    asio::io_service::strand m_ioStrand;
    bp::async_pipe m_outPipe;
    bp::async_pipe m_errorPipe;
    asio::streambuf m_outBuf;
    asio::streambuf m_errorBuf;
    std::string m_lineBuf;

    optional<bp::child> m_process;
    std::vector<std::string> m_command;
    bool m_awaitingExit = false;
};

template <typename Strand, typename Pipe, typename Buffer, typename Line>
static void
asyncReadLines (ProcessManager& manager, Strand& strand, Pipe& pipe,
                Buffer& buffer, Line& line, bs::error_code const& ec,
                std::size_t bytes) {
    std::istream stream (&buffer);
    if (ec || !bytes || !std::getline (stream, line)) {
        if (manager.awaitingExit ()) {
            manager.onUnexpectedExit ();
        } else {
            manager.onExit ();
        }
        return;
    }

    manager.onOutput (line);

    asio::async_read_until (
        pipe,
        buffer,
        '\n',
        strand.wrap (
            [&](boost::system::error_code const& ec, std::size_t bytes) {
                return asyncReadLines (
                    manager, strand, pipe, buffer, line, ec, bytes);
            }));
}

void
ProcessManagerImpl::start (ProcessManager& manager) {
    m_process.emplace (m_command,
                     bp::std_in.close (),
                     bp::std_out > m_outPipe,
                     bp::std_err > m_errorPipe);
    m_lineBuf.clear ();

    asio::async_read_until (
        m_outPipe,
        m_outBuf,
        '\n',
        m_ioStrand.wrap (
            [&](boost::system::error_code const& ec, std::size_t bytes) {
                return asyncReadLines (
                    manager, m_ioStrand, m_outPipe, m_outBuf, m_lineBuf, ec, bytes);
            }));

    asio::async_read_until (
        m_errorPipe,
        m_errorBuf,
        '\n',
        m_ioStrand.wrap (
            [&](boost::system::error_code const& ec, std::size_t bytes) {
                return asyncReadLines (
                    manager, m_ioStrand, m_errorPipe, m_errorBuf, m_lineBuf, ec, bytes);
            }));
}

template <typename T, typename... Args> static inline
std::unique_ptr<T>
make_unique (Args&&... args) {
    return std::unique_ptr<T>(new T (std::forward<Args>(args)...));
}

ProcessManager::ProcessManager (asio::io_service &io)
    : m_mainIoService (io), m_impl (make_unique<ProcessManagerImpl> (m_mainIoService)) {
}

ProcessManager::~ProcessManager () noexcept {
}

void
ProcessManager::start (std::vector<std::string> command) {
    std::unique_lock<std::mutex> lock (m_impl->mtx);
    if (m_impl->m_process) {
        throw std::logic_error (
            "ProcessManager::start() called while process is running");
    }

    m_impl->m_command = move (command);
    m_impl->start (*this);
}

bool
ProcessManager::awaitingExit () const noexcept {
    std::unique_lock<std::mutex> lock (m_impl->mtx);
    return m_impl->m_awaitingExit;
}

void ProcessManager::shutdown()
{

}
