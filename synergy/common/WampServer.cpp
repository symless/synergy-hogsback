#include <synergy/common/WampServer.h>

namespace ip = boost::asio::ip;
using ip::tcp;

static bool const debug = true;

WampServer::WampServer (boost::asio::io_service& io):
    m_executor (io),
    m_session (std::make_shared<autobahn::wamp_session>(io, debug))
{
}

void
WampServer::start (std::string const& ip, int const port)
{
    if (m_transport) {
        m_transport->disconnect().get();
        m_transport->detach();
        m_transport.reset();
    }

    m_transport = std::make_shared<autobahn::wamp_tcp_transport>
        (ioService(), tcp::endpoint(ip::address_v4::from_string(ip), port), debug);
    m_transport->attach
        (std::static_pointer_cast<autobahn::wamp_transport_handler>(m_session));

    m_transport->connect().then (m_executor, [&](boost::future<void> connected) {
        connected.get();
        m_session->start().then(m_executor, [&](boost::future<void> started) {
            started.get();
            m_session->join("default").then
                (m_executor, [&](boost::future<uint64_t> joined) {
                joined.get();
                ready();

                this->provide(kKeepAliveFunction, [this]() { });
            });
        });
    });
}
