#include <synergy/common/WampClient.h>

static bool const debug = true;

WampClient::WampClient(boost::asio::io_service& io):
    m_executor (io),
    m_session (std::make_shared<autobahn::wamp_session>(ioService(), debug))
{
    m_default_call_options.set_timeout (std::chrono::seconds(10));
}

boost::future<void>
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

    return m_transport->connect().then (m_executor,
                                      [&](boost::future<void> connected) {
        connected.get();
        m_session->start().then (m_executor, [&](boost::future<void> started) {
            started.get();
            m_session->join("default").then
                    (m_executor, [&](boost::future<uint64_t> joined) {
                joined.get();
                ready();
            });
        });
    });
}
