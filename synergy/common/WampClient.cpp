#include <synergy/common/WampClient.h>

static bool const kDebugWampClient = false;
static boost::posix_time::seconds const kRPCKeepAliveInterval (3);

WampClient::WampClient (boost::asio::io_service& ioService,
                        std::shared_ptr<spdlog::logger> logger):
    m_executor (ioService),
    m_keepAliveTimer (ioService),
    m_logger (std::move (logger))
{
    m_defaultCallOptions.set_timeout (std::chrono::seconds (3));

    connected.connect ([this](){
        this->keepAlive();
    });

    disconnected.connect ([this]() {
        m_connected = false;
        m_keepAliveTimer.cancel();
        this->log()->error ("RPC connection failed");
    });
}

bool
WampClient::isConnected() const {
    return m_connected;
}

void
WampClient::disconnect () {
    m_connected = false;
    boost::system::error_code ec;
    m_keepAliveTimer.cancel(ec);

    if (m_session) {
        m_session->leave ("");
        m_session->stop ();
    }

    ioService().poll();

    if (m_transport) {
        m_transport->disconnect().get();
        m_transport->detach();
        m_session.reset();
        m_transport.reset();
    }

    ioService().poll();
}

void
WampClient::connect (std::string const& ip, int const port) {
    auto endpoint = boost::asio::ip::tcp::endpoint
                    (boost::asio::ip::address::from_string(ip), port);
    disconnect();

    m_transport = std::make_shared<autobahn::wamp_tcp_transport>
        (ioService(), endpoint, kDebugWampClient);

    m_session = std::make_shared<autobahn::wamp_session> (ioService(),
                                                          kDebugWampClient);

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
            disconnected();
            return;
        }
        m_session->start().then(m_executor, [&](boost::future<void> started) {
            try {
                started.get();
            } catch (const std::exception& e) {
                log()->error ("RPC start() failed: {}", e.what());
                disconnected();
                return;
            }
            m_session->join("default").then(m_executor, [&](boost::future<uint64_t> joined) {
                try {
                    joined.get();
                } catch (const std::exception& e) {
                    log()->error ("RPC join() failed: {}", e.what());
                    disconnected();
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
    m_keepAliveTimer.async_wait ([this](auto const errorCode) {
        if (errorCode == boost::asio::error::operation_aborted) {
            return;
        } else if (errorCode || !this->isConnected()) {
            this->log()->error ("RPC keepalive failed");
            return;
        }

        this->call<void>("synergy.keepalive");
        this->keepAlive();
    });
}
