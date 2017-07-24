#include "WampRouter.h"

#include <bonefish/serialization/wamp_serializers.hpp>
#include <bonefish/serialization/json_serializer.hpp>
#include <bonefish/serialization/msgpack_serializer.hpp>
#include <bonefish/router/wamp_router.hpp>
#include <bonefish/router/wamp_routers.hpp>
#include <bonefish/rawsocket/rawsocket_server.hpp>
#include <bonefish/rawsocket/tcp_listener.hpp>
#include <bonefish/trace/trace.hpp>
#include <bonefish/websocket/websocket_server.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/bind.hpp>
#include <signal.h>
#include <string.h>

using namespace bonefish;

WampRouter::WampRouter(boost::asio::io_service &ioService, std::string ip, int port) :
    m_ioService(ioService),
    m_routers(std::make_shared<wamp_routers>()),
    m_serializers(std::make_shared<wamp_serializers>()),
    m_rawsocketServer(),
    m_websocketServer(),
    m_websocketPort(0)
{
    // Turn on bonefish tracing if requested. This may also turn on
    // lower level logging in third-party dependencies of bonefish
    // such as websocketpp.
    bonefish::trace::set_enabled(true);

    auto router = std::make_shared<wamp_router>(m_ioService, "default");
    m_routers->add_router(router);

//  m_serializers->add_serializer(std::make_shared<json_serializer>());

    m_serializers->add_serializer(std::make_shared<msgpack_serializer>());

//    if (m_websocket_server) {
//        m_websocket_server = std::make_shared<websocket_server>(
//                m_io_service, m_routers, m_serializers);
//    }

    m_rawsocketServer = std::make_shared<rawsocket_server>(m_routers, m_serializers);

    auto listener = std::make_shared<tcp_listener>(
            m_ioService, boost::asio::ip::address_v4::from_string(ip), port);
    m_rawsocketServer->attach_listener(std::static_pointer_cast<rawsocket_listener>(listener));
}

WampRouter::~WampRouter()
{
}

void
WampRouter::start()
{
    if (m_rawsocketServer) {
        m_rawsocketServer->start();
    }

    if (m_websocketServer) {
        m_websocketServer->start(boost::asio::ip::address(), m_websocketPort);
    }

    ready();
}

void
WampRouter::stop()
{
    if (m_websocketServer) {
        m_websocketServer->shutdown();
    }

    if (m_rawsocketServer) {
        m_rawsocketServer->shutdown();
    }
}
