#include <synergy/common/WampClient.h>
#include <synergy/common/SocketOptions.hpp>

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
        assert (this->m_connected);
    });

    disconnected.connect ([this](bool) {
        this->m_connected = false;

        boost::system::error_code ec;
        this->m_keepAliveTimer.cancel(ec);
        this->ioService().poll();

        this->log()->info ("RPC disconnected");
    });
}

bool
WampClient::isConnected() const {
    return m_connected;
}

void
WampClient::disconnect () {
    if (!this->m_connected) {
        return;
    }

    this->m_connected = false;
    disconnected(true);

    if (!m_session) {
        return;
    }

    m_session->leave ("").then (this->executor(), [this](auto left) {
        left.get();

        this->m_session->stop ().then (this->executor(), [this](auto stopped) {
            stopped.get();

            if (!this->m_transport) {
                this->ioService().poll();
                this->m_session.reset();
                return;
            }

            this->m_transport->disconnect().then(this->executor(),
                                           [this](auto disconnected) {
                disconnected.get();
                this->m_transport->detach();
                this->m_transport.reset();
                // TODO: why can't we do this?
                // this->m_session.reset();
            });
        });
    });
}

void
WampClient::connect (std::string const& ip, int const port) {
    if (this->m_connected) {
        return;
    }

    auto endpoint = boost::asio::ip::tcp::endpoint
                    (boost::asio::ip::address::from_string(ip), port);

    auto transport = std::make_shared<autobahn::wamp_tcp_transport>
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
            disconnected(false);
            return;
        }

        auto transport = std::static_pointer_cast<autobahn::wamp_tcp_transport>
                            (m_transport);
        boost::system::error_code ec;
        set_tcp_keep_alive_options (transport->socket (), 5, 1, 5);
        set_socket_to_close_on_exec (transport->socket (), ec);

        m_session->start().then(m_executor, [&](boost::future<void> started) {
            try {
                started.get();
            } catch (const std::exception& e) {
                log()->error ("RPC start() failed: {}", e.what());
                disconnected(false);
                return;
            }
            m_session->join("default").then(m_executor, [&](boost::future<uint64_t> joined) {
                try {
                    joined.get();
                } catch (const std::exception& e) {
                    log()->error ("RPC join() failed: {}", e.what());
                    disconnected(false);
                    return;
                }
                this->m_connected = true;
                this->connected();
            });
        });
    });

    // HACK
    // reason: We rely on this keep alive to detect initial connection failure
    // There should be a timer to do this
    keepAlive();
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
