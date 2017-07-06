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
        : ioStrand (io), outPipe (io), errorPipe (io) {
    }

    void start (ProcessManager& manager);

    std::mutex mtx;

    asio::io_service::strand ioStrand;
    bp::async_pipe outPipe;
    bp::async_pipe errorPipe;
    asio::streambuf outBuf;
    asio::streambuf errorBuf;
    std::string lineBuf;

    optional<bp::child> process;
    std::vector<std::string> command;
    bool awaitingExit = false;
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
    process.emplace (command,
                     bp::std_in.close (),
                     bp::std_out > outPipe,
                     bp::std_err > errorPipe);
    lineBuf.clear ();

    asio::async_read_until (
        outPipe,
        outBuf,
        '\n',
        ioStrand.wrap (
            [&](boost::system::error_code const& ec, std::size_t bytes) {
                return asyncReadLines (
                    manager, ioStrand, outPipe, outBuf, lineBuf, ec, bytes);
            }));

    asio::async_read_until (
        errorPipe,
        errorBuf,
        '\n',
        ioStrand.wrap (
            [&](boost::system::error_code const& ec, std::size_t bytes) {
                return asyncReadLines (
                    manager, ioStrand, errorPipe, errorBuf, lineBuf, ec, bytes);
            }));
}

template <typename T, typename... Args> static inline
std::unique_ptr<T>
make_unique (Args&&... args) {
    return std::unique_ptr<T>(new T (std::forward<Args>(args)...));
}

ProcessManager::ProcessManager (std::shared_ptr<asio::io_service> io)
    : m_io (move (io)), m_impl (make_unique<ProcessManagerImpl> (*m_io)) {
}

ProcessManager::~ProcessManager () noexcept {
}

void
ProcessManager::start (std::vector<std::string> command) {
    std::unique_lock<std::mutex> lock (m_impl->mtx);
    if (m_impl->process) {
        throw std::logic_error (
            "ProcessManager::start() called while process is running");
    }

    m_impl->command = move (command);
    m_impl->start (*this);
}

bool
ProcessManager::awaitingExit () const noexcept {
    std::unique_lock<std::mutex> lock (m_impl->mtx);
    return m_impl->awaitingExit;
}
