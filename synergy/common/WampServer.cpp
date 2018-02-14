#include <synergy/common/WampServer.h>

namespace ip = boost::asio::ip;
using ip::tcp;

static bool const kDebugWampServer = false;

WampServer::WampServer (boost::asio::io_service& ioService):
    m_executor (ioService),
    m_session (std::make_shared<autobahn::wamp_session>
               (ioService, kDebugWampServer)) {
}

void
WampServer::start (std::string const& ip, int const port) {
    if (m_transport) {
        m_transport->disconnect().get();
        m_transport->detach();
        m_transport.reset();
    }

    m_transport = std::make_shared<autobahn::wamp_tcp_transport>
        (ioService(), tcp::endpoint (ip::address::from_string(ip), port),
         kDebugWampServer);

    m_transport->attach
        (std::static_pointer_cast<autobahn::wamp_transport_handler>(m_session));

    m_transport->connect().then (m_executor, [&](boost::future<void> connected) {
        connected.get();

        m_session->start().then(m_executor, [&](boost::future<void> started) {
            started.get();

            m_session->join("default").then
                (m_executor, [&](boost::future<uint64_t> joined) {
                joined.get();
                this->provide ("synergy.network.noop", [](){}); // Removed in v2.0.6
                this->provide ("synergy.keepalive", [](){});
                ready();
            });
        });
    });
}
