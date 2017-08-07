#include <synergy/common/WampClient.h>
#include <iostream>

static bool const debug = true;

WampClient::WampClient(boost::asio::io_service& io):
    m_executor (io),
    m_session (std::make_shared<autobahn::wamp_session>(ioService(), debug)),
    m_retryTimer(io)
{
    m_defaultCallOptions.set_timeout (std::chrono::seconds(10));
}

void
WampClient::start (std::string const& ip, int const port)
{
    if (m_transport) {
        m_transport->disconnect().get();
        m_transport->detach();
        m_transport.reset();
    }

    m_transport = std::make_shared<autobahn::wamp_tcp_transport>(
                    ioService(), boost::asio::ip::tcp::endpoint
                        (boost::asio::ip::address_v4::from_string(ip), port),
                            debug);
    m_transport->attach(std::static_pointer_cast<autobahn::wamp_transport_handler>
                      (m_session));

    m_started = true;
    connect();
}

void
WampClient::connect()
{
    m_transport->connect().then(
        m_executor, [&](boost::future<void> status) {

        if (status.has_exception()) {
            if (!m_started) {
                return;
            }
            m_retryTimer.expires_from_now(boost::posix_time::milliseconds(500));
            m_retryTimer.async_wait([&](boost::system::error_code) {
                connectionRetry();
                connect();
            });
            return;
        }

        // if not connected, throws exception
        status.get();

        m_session->start().then (m_executor, [&](boost::future<void> started) {
            started.get();
            m_session->join("default").then
                    (m_executor, [&](boost::future<uint64_t> joined) {
                joined.get();
                connected();
            });
        });
    });
}
