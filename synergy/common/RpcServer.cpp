#include <synergy/common/RpcServer.h>

namespace ip = boost::asio::ip;
using ip::tcp;

RpcServer::RpcServer
(boost::asio::io_service& io, std::string ipAddress, int const port):
    m_executor (io)
{
    bool const debug = true;
    m_session = std::make_shared<autobahn::wamp_session>(io, debug);

    m_transport = std::make_shared<autobahn::wamp_tcp_transport>
        (io, tcp::endpoint(ip::address_v4::from_string(ipAddress), port),
         debug);
    m_transport->attach
        (std::static_pointer_cast<autobahn::wamp_transport_handler>(m_session));
}

void
RpcServer::start()
{
    m_transport->connect().then (m_executor, [&](boost::future<void> connected) {
        connected.get();
        m_session->start().then(m_executor, [&](boost::future<void> started) {
            started.get();
            m_session->join("default").then
                (m_executor, [&](boost::future<uint64_t> joined) {
                joined.get();
                ready();
            });
        });
    });
}
