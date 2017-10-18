#include "ClientProxy.hpp"

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

class ClientProxyMessageHandler {
public:
    explicit ClientProxyMessageHandler (ClientProxy& proxy) : proxy_ (proxy) {
    }
    ClientProxy&
    proxy () const {
        return proxy_;
    }
    void operator() (Message const&, int32_t source) const;

    void handle (ProxyClientConnect const&, int32_t source) const;
    void handle (CoreMessage const&, int32_t source) const;
    template <typename T>
    void
    handle (T const&, int32_t source) const {
    }

private:
    ClientProxy& proxy_;
};

class ClientProxyConnection {
public:
    explicit ClientProxyConnection (tcp::socket socket, int32_t client_id,
                                    std::string screen_name)
        : client_id_ (client_id),
          socket_ (std::move (socket)),
          screen_name_ (screen_name) {
    }

    void start (ClientProxy& proxy);
    void write (std::vector<uint8_t> const& data);

    int32_t client_id_;
    tcp::socket socket_;
    std::string screen_name_;
};

ClientProxy::ClientProxy (asio::io_service& io, Router& router, int port)
    : io_ (io), port_ (port), router_ (router) {
    message_handler_ = std::make_unique<ClientProxyMessageHandler> (*this);
    router_.on_receive.connect (*message_handler_);
}

ClientProxy::~ClientProxy () {
}

void
ClientProxy::start () {
}

void
ClientProxy::connect (int32_t client_id, const std::string& screen_name) {
    /* Connect loop */
    asio::spawn (io_, [this, client_id, screen_name](auto ctx) {
        tcp::socket socket (io_);
        boost::system::error_code ec;
        socket.open (tcp::v4 ());
        set_tcp_socket_buffer_sizes (socket, ec);
        socket.async_connect (
            tcp::endpoint (ip::address_v4::from_string ("127.0.0.1"), port_),
            ctx[ec]);

        if (ec) {
            asio::steady_timer timer (io_);
            timer.expires_from_now (std::chrono::seconds (3));
            timer.async_wait (ctx[ec]);
        } else {
            socket.set_option (tcp::no_delay (true), ec);
            connections_.emplace_back (std::make_shared<ClientProxyConnection> (
                std::move (socket), client_id, std::move (screen_name)));
            connections_.back ()->start (*this);
        }
    });
}

Router&
ClientProxy::router () const {
    return router_;
}

void
ClientProxyConnection::start (ClientProxy& proxy) {
    /* Read loop */
    asio::spawn (socket_.get_io_service (), [this, &proxy](auto ctx) {
        std::vector<unsigned char> buffer;
        int32_t size = 0;
        boost::system::error_code ec;
        synergy::protocol::v1::Handler handler (proxy.router (), client_id_);
        handler.on_hello.connect ([this]() {
            synergy::protocol::v1::HelloBackMessage helloBack;
            helloBack.args ().version.major = 1;
            helloBack.args ().version.minor = 6;
            helloBack.args ().screen_name   = screen_name_;

            std::vector<unsigned char> buffer;
            int32_t size = helloBack.size ();
            buffer.resize (size);
            helloBack.write_to (reinterpret_cast<char*> (buffer.data ()));

            this->write (buffer);
        });


        buffer.reserve (64 * 1024);

        while (true) {
            asio::async_read (socket_,
                              asio::buffer (&size, sizeof (size)),
                              asio::transfer_exactly (sizeof (size)),
                              ctx[ec]);

            // TODO: cleanup connections in ClientProxy
            if (ec) {
                throw boost::system::system_error (ec, ec.message ());
            }

            boost::endian::big_to_native_inplace (size);
            buffer.resize (size);
            asio::async_read (socket_,
                              asio::buffer (buffer),
                              asio::transfer_exactly (size),
                              ctx[ec]);
            if (ec) {
                throw boost::system::system_error (ec, ec.message ());
            }

            synergy::protocol::v1::parse (
                synergy::protocol::v1::Flow::STC,
                handler,
                reinterpret_cast<char*> (buffer.data ()),
                size);
        }
    });
}

void
ClientProxyConnection::write (const std::vector<uint8_t>& data) {
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
ClientProxyMessageHandler::
operator() (Message const& message, int32_t source) const {
    boost::apply_visitor (
        [this, source](auto& body) { this->handle (body, source); },
        message.body ());
}

void
ClientProxyMessageHandler::handle (ProxyClientConnect const& pcc,
                                   int32_t source) const {
    routerLog ()->info (
        "ClientProxy: received client connection for {} from screen",
        pcc.screen,
        source);

    auto& connections = proxy ().connections_;
    auto it           = std::find_if (
        begin (connections), end (connections), [source](auto& connection) {
            return connection->client_id_ == source;
        });

    if (it == end (connections)) {
        proxy ().connect (source, pcc.screen);
    }
}

void
ClientProxyMessageHandler::handle (const CoreMessage& cm,
                                   int32_t source) const {
    auto& connections = proxy ().connections_;
    auto it           = std::find_if (
        begin (connections), end (connections), [source](auto& connection) {
            return connection->client_id_ == source;
        });

    if (it == end (connections)) {
        routerLog ()->info ("ClientProxy: received core message from client "
                            "{}, but server has not established a connect to "
                            "that client",
                            source);
        return;
    }

    auto& c = *it;
    c->write (cm.data);
}
