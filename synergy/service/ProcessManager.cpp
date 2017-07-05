#include <synergy/service/ProcessManager.h>
#include <boost/process.hpp>
#include <boost/process/async_pipe.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/optional.hpp>
#include <vector>
#include <string>

namespace bp = boost::process;
namespace bs = boost::system;
using boost::optional;

class ProcessManagerImpl {
public:
    ProcessManagerImpl(asio::io_service& io):
        outPipe(io), errorPipe(io) {
    }

    void start (ProcessManager& manager);

    // TODO: add locking
    // TODO: put both pipes on the same asio strand & share the line buffer
    bp::async_pipe outPipe;
    asio::streambuf outBuf;
    std::string outLine;

    bp::async_pipe errorPipe;
    asio::streambuf errorBuf;
    std::string errorLine;

    optional<bp::child> process;
    std::vector<std::string> command;
    bool awaitingExit = false;
};

template <typename Pipe, typename Buffer, typename Line> static
void
asyncReadLines (ProcessManager& manager,
         Pipe& pipe, Buffer& buffer, Line& line,
         bs::error_code const& ec, std::size_t bytes)
{
    std::istream stream (&buffer);
    if (ec || !bytes || !std::getline (stream, line)) {
        if (manager.awaitingExit()) {
            manager.onUnexpectedExit();
        } else {
            manager.onExit();
        }
        return;
    }
    manager.onOutput (line);

    asio::async_read_until (pipe, buffer, '\n',
        [&](boost::system::error_code const& ec, std::size_t bytes) {
            return asyncReadLines (manager, pipe, buffer, line, ec, bytes);
    });
}

void
ProcessManagerImpl::start (ProcessManager& manager)
{
    process.emplace (command, bp::std_in.close (), bp::std_out > outPipe,
                     bp::std_err > errorPipe);
    outLine.clear();
    errorLine.clear();
    // TODO: clear buffers

    asio::async_read_until (outPipe, outBuf, '\n',
        [&](boost::system::error_code const& ec, std::size_t bytes) {
            return asyncReadLines (manager, outPipe, outBuf, outLine, ec, bytes);
    });

    asio::async_read_until (errorPipe, errorBuf, '\n',
        [&](boost::system::error_code const& ec, std::size_t bytes) {
            return asyncReadLines (manager, errorPipe, errorBuf, errorLine, ec, bytes);
    });
}

ProcessManager::ProcessManager
(std::shared_ptr<asio::io_service> io):
    m_io (move (io)),
    m_impl (std::make_unique<ProcessManagerImpl>(*m_io))
{
}

ProcessManager::~ProcessManager() noexcept
{}

void
ProcessManager::start
(std::vector<std::string> command)
{
    if (m_impl->process) {
        throw std::logic_error ("ProcessManager::start() called while process is running");
    }

    m_impl->command = move (command);
    m_impl->start (*this);
}

bool
ProcessManager::awaitingExit() const noexcept
{
    return m_impl->awaitingExit; // note: not thread safe
}
