#include <synergy/service/CoreProcess.h>
#include <synergy/service/CoreProcessImpl.h>
#include <synergy/service/ServiceLogs.h>
#include <boost/algorithm/string/trim.hpp>

namespace bs = boost::system;
namespace bp = boost::process;
static auto const kConnectingTimeout = std::chrono::seconds (3);

template <typename Pipe, typename Buffer, typename Line> static
void
asyncReadLines (CoreProcess& interface, Pipe& pipe, Buffer& buffer,
                Line& line, bs::error_code const& ec, std::size_t const bytes) {
    std::istream stream (&buffer);
    if (ec || !bytes || !std::getline (stream, line)) {
        return;
    }

    boost::algorithm::trim_right(line);
    interface.output (line);

    boost::asio::async_read_until (
        pipe,
        buffer,
        '\n',
        [&](boost::system::error_code const& ec, std::size_t const bytes) {
            return asyncReadLines (interface, pipe, buffer, line, ec, bytes);
        }
    );
}

CoreProcessImpl::CoreProcessImpl (
    CoreProcess& interface,
    boost::asio::io_service& io,
    std::vector<std::string> command
):  m_interface(interface),
    m_command(std::move(command)),
    m_outPipe (io),
    m_errorPipe (io),
    m_connectionTimer(io) {
}

CoreProcessImpl::~CoreProcessImpl() noexcept = default;

void
CoreProcessImpl::start () {
    assert (!m_command.empty());
    assert (!m_process);

    m_process.emplace (
        m_command,
        bp::std_in.close(),
        bp::std_out > m_outPipe,
        bp::std_err > m_errorPipe,
        bp::on_exit = [this](int, std::error_code const&) {
            serviceLog()->debug("core process exited: code={} expected={}", exit, m_expectingExit);

            try {
                m_process->wait();
            } catch (const std::exception& ex) {
                serviceLog()->error("can't wait() on core process: {}",
                                    ex.what());
            } catch (...) {
                serviceLog()->error("can't wait() on core process: unknown exception");
            }

            if (m_expectingExit) {
                return m_interface.expectedExit();
            } else {
                /* Cancel the standard stream I/O loops */
                boost::system::error_code ignored_ec;
                m_connectionTimer.cancel(ignored_ec);
                m_outPipe.cancel ();
                m_errorPipe.cancel ();
                getIoService().poll();
                serviceLog()->debug("core process I/O cancelled");

                /* Disconnect internal signal handling */
                for (auto& signal: m_signals) {
                    signal.disconnect();
                }
                m_signals.clear();
                serviceLog()->debug("core process event handlers disconnected");

                return m_interface.unexpectedExit();
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
    if (m_expectingExit) {
        serviceLog()->debug("ignoring duplicate core process shutdown request");
        return;
    }

    serviceLog()->debug("shutting down core process...");
    m_expectingExit = true;

    /* Cancel the standard stream I/O loops */
    boost::system::error_code ignored_ec;
    m_connectionTimer.cancel(ignored_ec);
    m_outPipe.cancel ();
    m_errorPipe.cancel ();
    getIoService().poll();
    serviceLog()->debug("core process I/O cancelled");

    /* Disconnect internal signal handling */
    for (auto& signal: m_signals) {
        signal.disconnect();
    }
    m_signals.clear();
    serviceLog()->debug("core process event handlers disconnected");

    m_process->terminate();
}

boost::asio::io_service&
CoreProcessImpl::getIoService() {
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
        timer.async_wait ([this, screenName = std::move(screenName)]
                          (auto const ec) {
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
