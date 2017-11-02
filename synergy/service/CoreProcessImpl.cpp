#include <synergy/service/CoreProcess.h>
#include <synergy/service/CoreProcessImpl.h>
#include <synergy/service/ServiceLogs.h>
#include <boost/algorithm/string/trim.hpp>

namespace bs = boost::system;
namespace bp = boost::process;
static auto const kConnectingTimeout = std::chrono::seconds (3);

template <typename Pipe, typename Buffer, typename Line> static
void
asyncReadLines (CoreProcess& manager, Pipe& pipe, Buffer& buffer,
                Line& line, bs::error_code const& ec, std::size_t const bytes) {
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
        [&](boost::system::error_code const& ec, std::size_t const bytes) {
            return asyncReadLines (manager, pipe, buffer, line, ec, bytes);
        }
    );
}

CoreProcessImpl::CoreProcessImpl (
    CoreProcess& interface,
    boost::asio::io_service& io,
    std::vector<std::string> command
):  m_interface(interface),
    m_command(std::move(command)),
    m_connectionTimer(io),
    m_outPipe (io),
    m_errorPipe (io) {
}

CoreProcessImpl::~CoreProcessImpl() noexcept {
    boost::system::error_code ec;
    m_outPipe.close(ec);
    m_errorPipe.close(ec);
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
                m_interface.expectedExit();
            } else {
                m_interface.unexpectedExit();
            }
        },
        getIoService()
    );

    /* Start the stdout I/O loop */
    boost::asio::async_read_until (
        m_outPipe,
        m_outputBuffer,
        '\n',
        [&](boost::system::error_code const& ec, std::size_t bytes) {
            return asyncReadLines (m_interface, m_outPipe, m_outputBuffer,
                                   m_lineBuffer, ec, bytes);
        }
    );

    /* Start the stderr I/O loop */
    boost::asio::async_read_until (
        m_errorPipe,
        m_errorBuffer,
        '\n',
        [&](boost::system::error_code const& ec, std::size_t bytes) {
            return asyncReadLines (m_interface, m_errorPipe, m_errorBuffer,
                                   m_lineBuffer, ec, bytes);
        }
    );

    serviceLog()->debug("core process started, id={}", m_process->id());
}


void
CoreProcessImpl::shutdown()
{
    serviceLog()->debug("stopping core process");
    auto& ioService = getIoService();

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

boost::asio::io_service&
CoreProcessImpl::getIoService()
{
    return m_connectionTimer.get_io_service();
}

void
CoreProcessImpl::onScreenStatusChanged (
    std::string screenName,
    ScreenStatus const screenStatus
){
    auto& timer = m_connectionTimer;
    if (screenStatus == ScreenStatus::kConnecting) {
        timer.expires_from_now (kConnectingTimeout);
        timer.async_wait ([this, screenName = std::move(screenName)](auto const ec) {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            } else if (ec) {
                throw boost::system::system_error (ec, ec.message());
            }
            m_interface.screenConnectionError (std::move (screenName));
        });
    } else {
        boost::system::error_code ec;
        timer.cancel (ec);
    }
}
