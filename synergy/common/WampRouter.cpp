#include <synergy/common/WampRouter.h>
#include <synergy/common/SocketOptions.hpp>
#include <bonefish/serialization/wamp_serializers.hpp>
#include <bonefish/serialization/msgpack_serializer.hpp>
#include <bonefish/router/wamp_router.hpp>
#include <bonefish/router/wamp_routers.hpp>
#include <bonefish/rawsocket/rawsocket_server.hpp>
#include <bonefish/rawsocket/tcp_listener.hpp>
#include <bonefish/trace/trace.hpp>
#include <boost/asio/ip/address.hpp>

static bool const kDebugWampRouter = false;

WampRouter::WampRouter (boost::asio::io_service& ioService,
                        std::shared_ptr<spdlog::logger> log):
    m_ioService(ioService),
    m_log (std::move (log)),
    m_routers(std::make_shared<bonefish::wamp_routers>()),
    m_serializers(std::make_shared<bonefish::wamp_serializers>())
{
    bonefish::trace::set_enabled (kDebugWampRouter);

    m_routers->add_router
        (std::make_shared<bonefish::wamp_router>(ioService, "default"));

    m_serializers->add_serializer
        (std::make_shared<bonefish::msgpack_serializer>());

    m_rawsocketServer = std::make_shared<bonefish::rawsocket_server>
        (m_routers, m_serializers);
}

WampRouter::~WampRouter() {
}

void
WampRouter::start (std::string const& ip, int const port)  {
    auto listener = std::make_shared<bonefish::tcp_listener>
        (ioService(), boost::asio::ip::address::from_string(ip), port);

    m_rawsocketServer->attach_listener
        (std::static_pointer_cast<bonefish::rawsocket_listener> (listener));

    m_rawsocketServer->start();

    boost::system::error_code ec;
    if (!set_socket_to_close_on_exec (listener->acceptor(), ec)) {
        log()->critical ("Failed to set RPC router port to close-on-exec: {}",
                          ec.message());
    }

    ready();
}

void
WampRouter::stop() {
    m_rawsocketServer->shutdown();
}
