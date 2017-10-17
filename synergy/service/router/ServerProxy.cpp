#include "ServerProxy.hpp"

#include <synergy/service/router/Router.hpp>
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

struct ServerProxyMessageHandler {
    explicit ServerProxyMessageHandler (ServerProxy& proxy): proxy_(proxy) {}

public:
    ServerProxy& proxy() const { return proxy_; }
    void operator()(Message const&, int32_t source) const;
    void handle(ProxyServerClaim const&psc, int32_t source) const;
    void handle(CoreMessage const&cm, int32_t source) const;
    template <typename T> void handle (T const&, int32_t source) const {}

private:
    ServerProxy& proxy_;

};

struct ServerProxyConnection {
    explicit ServerProxyConnection (asio::io_service& io) : id_ (0), socket_ (io) {
    }

    void start (ServerProxy &proxy);
    void write(std::vector<uint8_t>const &data);

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(std::string screenname)> on_hello_back;

    int32_t id_;
    tcp::socket socket_;
};

ServerProxy::ServerProxy (asio::io_service& io, Router& router, int const port) :
    acceptor_ (io),
    router_(router),
    server_id_(3)
{
    tcp::endpoint endpoint (ip::address (), port);
    acceptor_.open (endpoint.protocol ());
    acceptor_.set_option (tcp::socket::reuse_address (true));
    acceptor_.bind (endpoint);

    /* On individual connections, the socket buffer size must be set prior to
     * the listen(2) or connect(2) calls in order to have it take effect */
    boost::system::error_code ec;
    set_tcp_socket_buffer_sizes (acceptor_, ec);
    acceptor_.listen ();

    message_handler_ = std::make_unique<ServerProxyMessageHandler>(*this);
    router_.on_receive.connect (*message_handler_);
}

ServerProxy::~ServerProxy()
{

}

void
ServerProxy::
start () {
    asio::spawn (acceptor_.get_io_service (), [this](auto ctx) {
        while (true) {
            auto connection = std::make_shared<ServerProxyConnection> (
                        acceptor_.get_io_service ());

            boost::system::error_code ec;
            acceptor_.async_accept (connection->socket_, ctx[ec]);
            if (ec) {
                throw boost::system::system_error (ec, ec.message ());
            }
            connection->on_hello_back.connect([this](std::string screenname){
                ProxyClientConnect pcc;
                pcc.screen = std::move(screenname);
                router_.send(pcc, server_id_);
            });

            connections_.push_back (connection);
            connection->start (*this);
        }
    });
}

Router &ServerProxy::router() const
{
    return router_;
}

uint32_t ServerProxy::server_id() const
{
    return server_id_;
}

void
ServerProxyConnection::start (ServerProxy& proxy) {
    /* Read loop */
    asio::spawn (socket_.get_io_service (), [this, &proxy](auto ctx) {
        std::vector<unsigned char> buffer;
        buffer.reserve (64 * 1024);

        routerLog()->debug ("sending hello to core client");

        /* Get the ball rolling */
        synergy::protocol::v1::HelloMessage hello;
        hello.args ().version.major = 1;
        hello.args ().version.minor = 6;

        int32_t size = hello.size ();
        buffer.resize (size);
        boost::endian::big_to_native_inplace (size);
        hello.write_to (reinterpret_cast<char*>(buffer.data ()));

        std::array<asio::const_buffer, 2> const buffers = {
            asio::const_buffer (&size, sizeof (size)),
            asio::const_buffer (buffer.data (), buffer.size ())};

        boost::system::error_code ec;
        asio::async_write (socket_, buffers, ctx[ec]);
        synergy::protocol::v1::Handler handler(proxy.router(), proxy.server_id());

        handler.on_hello_back.connect([this](std::string screenname){
            on_hello_back(screenname);
        });

        while (true) {
            asio::async_read (socket_,
                              asio::buffer (&size, sizeof (size)),
                              asio::transfer_exactly (sizeof (size)),
                              ctx[ec]);
            if (ec) {
                // TODO: detect disconnect and remove this connection from array
                routerLog()->debug ("Error reading from core client: {}", ec.message());
                break;
            }

            boost::endian::big_to_native_inplace (size);
            buffer.resize (size);

            asio::async_read (socket_,
                              asio::buffer (buffer),
                              asio::transfer_exactly (size),
                              ctx[ec]);
            if (ec) {
                // TODO: detect disconnect and remove this connection from array
                routerLog()->debug ("Error reading from core client: {}", ec.message());
                break;
            }

            try {
                synergy::protocol::v1::parse (
                            synergy::protocol::v1::Flow::CTS, handler, reinterpret_cast<char*>(buffer.data ()), size);
            }
            catch (const std::exception& e) {
                routerLog()->error ("ServerProxy: failed to parse message: {}",
                                   e.what());
            }
        }
    });
}

void ServerProxyConnection::write(const std::vector<uint8_t> &data)
{
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
ServerProxyMessageHandler::operator()(Message const& message, int32_t source) const
{
    boost::apply_visitor ([this, source](auto& body) {
        this->handle (body, source);
    }, message.body());
}

void ServerProxyMessageHandler::handle(const ProxyServerClaim &psc, int32_t source) const
{
    if (proxy_.server_id() != source) {
        proxy_.on_new_server(source);
    }
}

void ServerProxyMessageHandler::handle(const CoreMessage& cm, int32_t source) const
{
    if (!proxy_.connections_.empty()) {
        auto& connection = proxy_.connections_.back();
        connection->write(cm.data);
    }
}
