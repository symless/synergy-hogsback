#include <synergy/common/WampClient.h>
#include <iostream>

static bool const debug = true;

WampClient::WampClient(boost::asio::io_service& io):
    m_executor(io),
    m_session(std::make_shared<autobahn::wamp_session>(ioService(), debug))
{
    m_defaultCallOptions.set_timeout(std::chrono::seconds(2));
}

void
WampClient::start (std::string const& ip, int const port)
{
    if (m_transport) {
        m_transport->disconnect().get();
        m_transport->detach();
        m_transport.reset();
    }

    auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(ip), port);
    m_transport = std::make_shared<autobahn::wamp_tcp_transport>(ioService(), endpoint, debug);
    m_transport->attach(std::static_pointer_cast<autobahn::wamp_transport_handler>(m_session));

    connect();
}

void
WampClient::connect()
{
    connecting();
    m_transport->connect().then(m_executor, [&](boost::future<void> connected) {

        try {
            connected.get();
        }
        catch (const std::exception& e) {
            // BUG: when the connection fails, this only sometimes gets hit.
            // could be caused by a race condition? perhaps related to the cloud?
            commonLog()->error("rpc connect failed: {}", e.what());
            connectionError();
            return;
        }

        m_session->start().then(m_executor, [&](boost::future<void> started) {
            try {
                started.get();
            }
            catch (const std::exception& e) {
                commonLog()->error("rpc start failed: {}", e.what());
                connectionError();
                return;
            }

            m_session->join("default").then(m_executor, [&](boost::future<uint64_t> joined) {
                try {
                    joined.get();
                    this->connected();
                }
                catch (const std::exception& e) {
                    commonLog()->error("rpc join failed: {}", e.what());
                    connectionError();
                    return;
                }
            });
        });
    });
}
