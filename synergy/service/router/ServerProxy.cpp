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
#include <boost/numeric/conversion/cast.hpp>
#include <cstdint>
#include <synergy/service/ServiceLogs.h>
#include <synergy/service/router/Router.hpp>

class ServerProxyMessageHandler final {
public:
    explicit ServerProxyMessageHandler (ServerProxy& proxy);
    ServerProxy& proxy () const&;

    void operator() (Message const&, std::uint32_t source) const;
    void handle (CoreMessage const&, std::uint32_t source) const;
    void handle (ProxyServerClaim const&, std::uint32_t source) const;

    template <typename T>
    void handle (T const&,  std::uint32_t) const;

private:
    ServerProxy& proxy_;
};

class ServerProxyConnection:
    public std::enable_shared_from_this<ServerProxyConnection> {
public:
    explicit ServerProxyConnection (asio::io_service& io):
        socket_ (io) {
    }

    tcp::socket& socket () &;
    void start (ServerProxy& proxy, std::uint32_t server_id);
    void write (std::vector<uint8_t> data);
    void close ();

private:
    tcp::socket socket_;

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<bool(std::string screen_name), synergy::protocol::v1::all_true>
        on_hello_back;

    signal<void(std::shared_ptr<ServerProxyConnection> const&)>
        on_disconnect;
};


ServerProxy::ServerProxy (asio::io_service& io, Router& router,
                          uint16_t const port)
    : acceptor_ (io),
      router_ (router),
      message_handler_ (std::make_unique<ServerProxyMessageHandler> (*this)) {

    tcp::endpoint endpoint (ip::address::from_string("127.0.0.1"), port);
    acceptor_.open (endpoint.protocol ());
    acceptor_.set_option (tcp::socket::reuse_address (true));
    acceptor_.bind (endpoint);

    /* On individual connections, the socket buffer size must be set prior to
     * the listen(2) or connect(2) calls in order to have any effect
     */
    boost::system::error_code ec;
    restrict_tcp_socket_buffer_sizes (acceptor_, ec);
    acceptor_.listen ();
    router_.on_receive.connect (*message_handler_);
}

ServerProxy::~ServerProxy () noexcept = default;

void
ServerProxy::start (std::int64_t const server_id) {
    if (server_id < -1) {
        throw std::logic_error ("Invalid server ID passed to ServerProxy");
    } else if (server_id == this->server_id_) {
        return;
    } else if (server_id == -1) {
        this->stop();
        server_id_ = -1;
        return;
    }

    this->stop();
    server_id_ = boost::numeric_cast<std::uint32_t>(server_id);

    asio::spawn (acceptor_.get_io_service (), [this](auto ctx) {
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

            connection->on_hello_back.connect (
                [this](std::string screen_name) {
                    ProxyClientConnect pcc;
                    pcc.screen = std::move (screen_name);
                    return router().send (std::move (pcc), this->server_id_);
                });

            connection->on_disconnect.connect (
                [this](std::shared_ptr<ServerProxyConnection> const& connection) {
                    auto it = std::find (begin (connections_), end (connections_),
                                         connection);
                    if (it != end (connections_)) {
                        connections_.erase (it);
                    }
                });

            connections_.push_back (connection);
            connection->start (*this, boost::numeric_cast<std::uint32_t>(this->server_id_));
        }
    });
}

void
ServerProxy::stop () {
    acceptor_.cancel ();
    acceptor_.get_io_service().poll();
    for (auto& connection : connections_) {
        connection->close ();
    }
    acceptor_.get_io_service().poll();
    connections_.clear ();
}

void
ServerProxyConnection::start (ServerProxy& proxy,
                              std::uint32_t const server_id) {
    /* Read loop */
    asio::spawn (
        socket_.get_io_service (), [this, &proxy, server_id,
                                    self = this->shared_from_this()](auto ctx) {
            // HACK: wait for 200 ms, so core will be ready for this hello message
            asio::steady_timer timer (socket_.get_io_service());
            boost::system::error_code ec;
            timer.expires_from_now (std::chrono::milliseconds (200));
            timer.async_wait (ctx[ec]);

            std::vector<unsigned char> buffer;
            buffer.reserve (64 * 1024);

            synergy::protocol::v1::Handler handler (proxy.router (), server_id);
            handler.on_hello_back.connect (
                [this](std::string screen) {
                    return on_hello_back (std::move (screen));
                }
            );

            routerLog ()->debug ("Saying Hello to core client");
            synergy::protocol::v1::HelloMessage hello;
            hello.args ().version.major = 1;
            hello.args ().version.minor = 6;

            buffer.resize (hello.size ());
            uint32_t size = boost::numeric_cast<uint32_t>(hello.size ());
            boost::endian::native_to_big_inplace (size);
            hello.write_to (reinterpret_cast<char*> (buffer.data ()));

            std::array<asio::const_buffer, 2> const buffers = {
                asio::const_buffer (&size, sizeof (size)),
                asio::const_buffer (buffer.data (), buffer.size ())
            };

            asio::async_write (socket_, buffers, ctx[ec]);

            while (true) {
                asio::async_read (socket_,
                                  asio::buffer (&size, sizeof (size)),
                                  asio::transfer_exactly (sizeof (size)),
                                  ctx[ec]);
                if (ec) {
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
                    routerLog ()->debug ("Error reading from core client: {}",
                                         ec.message ());
                    break;
                }

                try {
                    if (!synergy::protocol::v1::process
                            (synergy::protocol::v1::Flow::CTS, handler,
                                buffer.data (), size)) {
                        break;
                    }
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

tcp::socket&
ServerProxyConnection::socket() & {
    return socket_;
}

void
ServerProxyConnection::close () {
    boost::system::error_code ec;
    socket().close (ec);
}

void
ServerProxyConnection::write (std::vector<uint8_t> data) {
    asio::spawn (socket_.get_io_service (),
                 [this, data = std::move(data)](auto ctx) {
        auto size = boost::numeric_cast<int32_t> (data.size ());
        boost::endian::native_to_big_inplace (size);

        std::array<asio::const_buffer, 2> const buffers = {
            asio::const_buffer (&size, sizeof (size)),
            asio::const_buffer (data.data (), data.size ())
        };

        boost::system::error_code ec;
        asio::async_write (socket_, buffers, ctx[ec]);
    });
}

ServerProxyMessageHandler::ServerProxyMessageHandler
(ServerProxy &proxy) : proxy_ (proxy) {
}

ServerProxy&
ServerProxyMessageHandler::proxy() const& {
    return proxy_;
}

void
ServerProxyMessageHandler::operator() (Message const& message,
                                       std::uint32_t const source) const {
    boost::apply_visitor (
        [this, source](auto& body) { this->handle (body, source); },
        message.body()
    );
}

void
ServerProxyMessageHandler::handle (ProxyServerClaim const& psc,
                                   std::uint32_t) const {
}

void
ServerProxyMessageHandler::handle (CoreMessage const& msg,
                                   std::uint32_t) const {
    if (!proxy().connections().empty ()) {
        auto& connection = proxy().connections().back ();
        connection->write (msg.data);
    }
}

template <typename T> inline
void
ServerProxyMessageHandler::handle (T const&, std::uint32_t) const {
}
