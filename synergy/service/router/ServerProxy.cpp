#include "ServerProxy.hpp"

#include "protocol/v1/MessageTypes.hpp"
#include "protocol/v1/Scanner.hpp"
#include "utils/tcp.hpp"
#include <array>
#include <boost/asio/buffer.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/endian/conversion.hpp>
#include <cstdint>
#include <synergy/service/ServiceLogs.h>
#include <synergy/service/router/Router.hpp>

class ServerProxyMessageHandler final {
public:
    explicit ServerProxyMessageHandler (ServerProxy& proxy) : proxy_ (proxy) {
    }

    ServerProxy&
    proxy () const {
        return proxy_;
    }

    template <typename T>
    void
    handle (T const&, int32_t) const {
    }

    void operator() (Message const&, int32_t source) const;
    void handle (CoreMessage const& cm, int32_t source) const;
    void handle (ProxyServerClaim const& psc, int32_t source) const;

private:
    ServerProxy& proxy_;
};

class ServerProxyConnection : public std::enable_shared_from_this<ServerProxyConnection> {
    friend class ServerProxy;

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    explicit ServerProxyConnection (asio::io_service& io)
        : id_ (0), socket_ (io) {
    }

    tcp::socket&
    socket () {
        return socket_;
    }

    void close ();
    void start (ServerProxy& proxy, int32_t server_id);
    void write (std::vector<uint8_t> const& data);

private:
    int32_t id_;
    tcp::socket socket_;

public:
    signal<void(std::string screen_name)> on_hello_back;
    signal<void(std::shared_ptr<ServerProxyConnection> const&)> on_disconnect;
};

ServerProxy::ServerProxy (asio::io_service& io, Router& router, int const port)
    : acceptor_ (io), router_ (router) {
    tcp::endpoint endpoint (ip::address (), port);
    acceptor_.open (endpoint.protocol ());
    acceptor_.set_option (tcp::socket::reuse_address (true));
    acceptor_.bind (endpoint);

    /* On individual connections, the socket buffer size must be set prior to
     * the listen(2) or connect(2) calls in order to have it take effect */
    boost::system::error_code ec;
    set_tcp_socket_buffer_sizes (acceptor_, ec);
    acceptor_.listen ();

    message_handler_ = std::make_unique<ServerProxyMessageHandler> (*this);
    router_.on_receive.connect (*message_handler_);
}

ServerProxy::~ServerProxy () {
}

void
ServerProxy::start (int32_t const server_id) {
    asio::spawn (acceptor_.get_io_service (), [this, server_id](auto ctx) {
        while (true) {
            auto connection = std::make_shared<ServerProxyConnection> (
                acceptor_.get_io_service ());

            boost::system::error_code ec;
            acceptor_.async_accept (connection->socket (), ctx[ec]);

            if (ec == boost::asio::error::operation_aborted) {
                return;
            } else if (ec) {
                throw boost::system::system_error (ec, ec.message ());
            }

            if (server_id < 0) {
                routerLog ()->error (
                    "Incoming core client connection but I have no server ID");
                connection->close ();
                continue;
            }

            connection->on_hello_back.connect (
                [this, server_id](std::string screenname) {
                    ProxyClientConnect pcc;
                    pcc.screen = std::move (screenname);
                    if (!router_.send (pcc, server_id)) {
                        // when it fails to send, drop the connection and let core client reconnect
                        connections_.back()->close();
                    }
                });

            connection->on_disconnect.connect (
                [this](
                    std::shared_ptr<ServerProxyConnection> const& connection) {
                    auto it = std::find (begin (connections_), end (connections_), connection);
                    if (it != end (connections_)) {
                        connections_.erase (it);
                    }
                });

            connections_.push_back (connection);
            connection->start (*this, server_id);
        }
    });
}

void
ServerProxy::reset () {
    acceptor_.cancel ();
    for (auto& connection : connections_) {
        connection->close ();
    }
    connections_.clear ();
}

Router&
ServerProxy::router () const {
    return router_;
}

void
ServerProxyConnection::close () {
    boost::system::error_code ec;
    socket_.close (ec);
}

void
ServerProxyConnection::start (ServerProxy& proxy, int32_t const server_id) {
    /* Read loop */
    asio::spawn (
        socket_.get_io_service (), [this, &proxy, server_id, self = shared_from_this()](auto ctx) {
            // HACK: wait for 200 ms, so core will be ready for this hello
            asio::steady_timer timer (socket_.get_io_service());
            boost::system::error_code ec;
            timer.expires_from_now (std::chrono::milliseconds (200));
            timer.async_wait (ctx[ec]);

            std::vector<unsigned char> buffer;
            buffer.reserve (64 * 1024);

            synergy::protocol::v1::Handler handler (proxy.router (), server_id);
            handler.on_hello_back.connect (
                [this](std::string screenname) { on_hello_back (screenname); });

            routerLog ()->debug ("Saying Hello to core client");
            synergy::protocol::v1::HelloMessage hello;
            hello.args ().version.major = 1;
            hello.args ().version.minor = 6;

            int32_t size = hello.size ();
            buffer.resize (size);
            boost::endian::big_to_native_inplace (size);
            hello.write_to (reinterpret_cast<char*> (buffer.data ()));

            std::array<asio::const_buffer, 2> const buffers = {
                asio::const_buffer (&size, sizeof (size)),
                asio::const_buffer (buffer.data (), buffer.size ())};

            asio::async_write (socket_, buffers, ctx[ec]);

            while (true) {
                asio::async_read (socket_,
                                  asio::buffer (&size, sizeof (size)),
                                  asio::transfer_exactly (sizeof (size)),
                                  ctx[ec]);
                if (ec) {
                    // TODO: detect disconnect and remove this connection from
                    // array
                    routerLog ()->debug ("Error reading from core client: {}",
                                         ec.message ());
                    break;
                }

                boost::endian::big_to_native_inplace (size);
                buffer.resize (size);

                asio::async_read (socket_,
                                  asio::buffer (buffer),
                                  asio::transfer_exactly (size),
                                  ctx[ec]);
                if (ec) {
                    // TODO: detect disconnect and remove this connection from
                    // array
                    routerLog ()->debug ("Error reading from core client: {}",
                                         ec.message ());
                    break;
                }

                try {
                    synergy::protocol::v1::process (
                                synergy::protocol::v1::Flow::CTS, handler, buffer.data (), size);
                }
                catch (const std::exception& e) {
                    routerLog()->error ("ServerProxy: failed to parse message: {}",
                                       e.what());
                }
            }

            routerLog ()->debug ("Terminating core client read loop");
            on_disconnect (self);
        });
}

void
ServerProxyConnection::write (const std::vector<uint8_t>& data) {
    asio::spawn (socket_.get_io_service (), [this, data](auto ctx) {
        int32_t size = data.size ();
        boost::endian::native_to_big_inplace (size);

        std::array<asio::const_buffer, 2> const buffers = {
            asio::const_buffer (&size, sizeof (size)),
            asio::const_buffer (data.data (), data.size ())};

        boost::system::error_code ec;
        asio::async_write (socket_, buffers, ctx[ec]);
    });
}

void
ServerProxyMessageHandler::
operator() (Message const& message, int32_t source) const {
    boost::apply_visitor (
        [this, source](auto& body) { this->handle (body, source); },
        message.body ());
}

void
ServerProxyMessageHandler::handle (const ProxyServerClaim& psc,
                                   int32_t source) const {
}

void
ServerProxyMessageHandler::handle (const CoreMessage& cm,
                                   int32_t source) const {
    if (!proxy_.connections_.empty ()) {
        auto& connection = proxy_.connections_.back ();
        connection->write (cm.data);
    }
}
