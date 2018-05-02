#include <synergy/common/WampServer.h>
#include <synergy/common/SocketOptions.hpp>

namespace ip = boost::asio::ip;
using ip::tcp;

static bool const kDebugWampServer = false;

WampServer::WampServer (boost::asio::io_service& ioService):
    m_executor (ioService),
    m_session (std::make_shared<autobahn::wamp_session> (ioService,
                                                         kDebugWampServer)) {
}

void
WampServer::start (std::string const& ip, int const port) {
    if (m_transport) {
        m_transport->disconnect().get();
        m_transport->detach();
        m_transport.reset();
    }

    m_transport = std::make_shared<autobahn::wamp_tcp_transport>
        (ioService(), tcp::endpoint (ip::address::from_string (ip), port),
            kDebugWampServer);

    m_transport->attach
        (std::static_pointer_cast<autobahn::wamp_transport_handler>(m_session));

    m_transport->connect().then(executor(), [&](boost::future<void> connected) {
        connected.get();

        auto transport = std::static_pointer_cast<autobahn::wamp_tcp_transport>
                            (m_transport);
        boost::system::error_code ec;
        set_tcp_keep_alive_options (transport->socket (), 5, 1, 5);
        set_socket_to_close_on_exec (transport->socket (), ec);
        m_transport = std::move(transport);

        m_session->start().then(executor(), [&](boost::future<void> started) {
            started.get();

            m_session->join("default").then(executor(),
                                        [&](boost::future<uint64_t> joined) {
                joined.get();
                this->provide ("synergy.keepalive", [](){});
                ready();
            });
        });
    });
}
