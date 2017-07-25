#include <synergy/common/WampRouter.h>
#include <bonefish/serialization/wamp_serializers.hpp>
#include <bonefish/serialization/msgpack_serializer.hpp>
#include <bonefish/router/wamp_router.hpp>
#include <bonefish/router/wamp_routers.hpp>
#include <bonefish/rawsocket/rawsocket_server.hpp>
#include <bonefish/rawsocket/tcp_listener.hpp>
#include <bonefish/trace/trace.hpp>
#include <boost/asio/ip/address.hpp>

static bool const debug = true;

WampRouter::WampRouter(boost::asio::io_service& ioService) :
    m_ioService(ioService),
    m_routers(std::make_shared<bonefish::wamp_routers>()),
    m_serializers(std::make_shared<bonefish::wamp_serializers>())
{
    bonefish::trace::set_enabled(debug);

    m_routers->add_router
        (std::make_shared<bonefish::wamp_router>(m_ioService, "default"));

    m_serializers->add_serializer
        (std::make_shared<bonefish::msgpack_serializer>());

    m_rawsocketServer = std::make_shared<bonefish::rawsocket_server>
        (m_routers, m_serializers);
}

WampRouter::~WampRouter()
{
}

void
WampRouter::start (std::string const& ip, int port)
{
    m_rawsocketServer->attach_listener
        (std::static_pointer_cast<bonefish::rawsocket_listener>
            (std::make_shared<bonefish::tcp_listener>
                (m_ioService, boost::asio::ip::address_v4::from_string(ip),
                    port)));
    m_rawsocketServer->start();
    ready();
}

void
WampRouter::stop()
{
    if (m_rawsocketServer) {
        m_rawsocketServer->shutdown();
    }
}
