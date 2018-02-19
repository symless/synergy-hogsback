#include <synergy/common/WampClient.h>

static bool const kDebugWampClient = true;
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

// TODO: make this return a future?
void
WampClient::disconnect () {
    m_connected = false;
    boost::system::error_code ec;
    m_keepAliveTimer.cancel(ec);

    disconnected();

    if (!m_session) {
        return;
    }

    m_session->leave ("").then (this->executor(), [this](auto left) {
        left.get();

        m_session->stop ().then (this->executor(), [this](auto stopped) {
            stopped.get();

            if (!m_transport) {
                ioService().poll();
                m_session.reset();
                return;
            }

            m_transport->disconnect().then(this->executor(),
                                           [this](auto disconnected) {
                disconnected.get();
                m_transport->detach();
                m_transport.reset();
                m_session.reset();
            });
        });
    });
}

void
WampClient::connect (std::string const& ip, int const port) {
    auto endpoint = boost::asio::ip::tcp::endpoint
                    (boost::asio::ip::address::from_string(ip), port);
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
