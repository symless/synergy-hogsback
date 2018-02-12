#include <synergy/common/WampClient.h>

static bool const kDebugWampClient = false;
static boost::posix_time::seconds const kRPCKeepAliveInterval (5);

WampClient::WampClient(boost::asio::io_service& ioService,
                       std::shared_ptr<spdlog::logger> log):
    m_executor (ioService),
    m_session (std::make_shared<autobahn::wamp_session>(ioService,
                                                        kDebugWampClient)),
    m_keepAliveTimer (ioService),
    m_log (log)
{
    m_defaultCallOptions.set_timeout (std::chrono::seconds (3));

    connectionError.connect ([this]() {
        this->m_connected = false;
        this->log()->error ("RPC connection failed");
    });

    connected.connect ([this](){
        this->keepAlive();
    });
}

bool
WampClient::isConnected() const {
    return m_connected;
}

void
WampClient::start (std::string const& ip, int const port) {
    auto endpoint = boost::asio::ip::tcp::endpoint
                    (boost::asio::ip::address_v4::from_string(ip), port);
    m_connected = false;

    if (m_transport) {
        m_transport->disconnect().get();
        m_transport->detach();
        m_transport.reset();
    }

    m_transport = std::make_shared<autobahn::wamp_tcp_transport>
        (ioService(), endpoint, kDebugWampClient);

    m_transport->attach
        (std::static_pointer_cast<autobahn::wamp_transport_handler>(m_session));

    connect();
}

void
WampClient::connect() {
    connecting();
    m_transport->connect().then(m_executor, [&](boost::future<void> connected) {
        try {
            connected.get();
        } catch (const std::exception& e) {
            log()->error ("RPC connect() failed: {}", e.what());
            connectionError();
            return;
        }
        m_session->start().then(m_executor, [&](boost::future<void> started) {
            try {
                started.get();
            } catch (const std::exception& e) {
                log()->error ("RPC start() failed: {}", e.what());
                connectionError();
                return;
            }
            m_session->join("default").then(m_executor, [&](boost::future<uint64_t> joined) {
                try {
                    joined.get();
                } catch (const std::exception& e) {
                    log()->error ("RPC join() failed: {}", e.what());
                    connectionError();
                    return;
                }
                m_connected = true;
                this->connected();
            });
        });
    });
}

void
WampClient::keepAlive() {
    m_keepAliveTimer.expires_from_now (kRPCKeepAliveInterval);
    m_keepAliveTimer.async_wait ([this](auto const ec) {
        if (ec || !this->isConnected()) {
            this->log ()->warn ("RPC keepalive disabled");
            return;
        }
        this->call<void>("synergy.noop");
        this->keepAlive();
    });
}
