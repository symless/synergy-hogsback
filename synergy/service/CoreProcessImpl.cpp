#include <synergy/service/CoreProcessImpl.h>

#include <synergy/service/CoreProcess.h>
#include <synergy/service/ServiceLogs.h>
#include <boost/algorithm/string/trim.hpp>

namespace bs = boost::system;
namespace bp = boost::process;

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
    m_io(io) {
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
        bp::on_exit = [this](int exit_code, std::error_code const&) {
            serviceLog()->debug("core process exited: code={} expected={}", exit_code,
                                 m_expectingExit);

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
                m_outPipe.cancel ();
                m_errorPipe.cancel ();
                getIoService().poll();
                serviceLog()->debug("core process I/O cancelled");

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
    m_outPipe.cancel ();
    m_errorPipe.cancel ();
    getIoService().poll();
    serviceLog()->debug("core process I/O cancelled");

    m_process->terminate();
}

boost::asio::io_service&
CoreProcessImpl::getIoService() {
    return m_io;
}
