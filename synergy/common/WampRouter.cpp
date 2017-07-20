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

WampRouter::WampRouter(boost::asio::io_service &ioService, std::string ip, int port)
    : m_ioService(ioService)
    , m_work()
#if defined(SIGQUIT)
    , m_termination_signals(m_ioService, SIGTERM, SIGINT, SIGQUIT)
#else
    , m_termination_signals(m_ioService, SIGTERM, SIGINT)
#endif
    , m_routers(std::make_shared<wamp_routers>())
    , m_serializers(std::make_shared<wamp_serializers>())
    , m_rawsocket_server()
    , m_websocket_server()
    , m_websocket_port(0)
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

    m_rawsocket_server = std::make_shared<rawsocket_server>(m_routers, m_serializers);

    auto listener = std::make_shared<tcp_listener>(
            m_ioService, boost::asio::ip::address_v4::from_string(ip), port);
    m_rawsocket_server->attach_listener(std::static_pointer_cast<rawsocket_listener>(listener));
}

WampRouter::~WampRouter()
{
    m_termination_signals.cancel();
}

void
WampRouter::run()
{
    m_termination_signals.async_wait(
            boost::bind(&WampRouter::termination_signal_handler, this, _1, _2));

    if (m_rawsocket_server) {
        m_rawsocket_server->start();
    }

    if (m_websocket_server) {
        m_websocket_server->start(boost::asio::ip::address(), m_websocket_port);
    }

    ready();
}

void
WampRouter::shutdown()
{
    if (m_work.get()) {
        m_ioService.dispatch(boost::bind(&WampRouter::shutdown_handler, this));
    }
}

void
WampRouter::shutdown_handler()
{
    if (m_work.get()) {
        m_termination_signals.cancel();

        if (m_websocket_server) {
            m_websocket_server->shutdown();
        }
        if (m_rawsocket_server) {
            m_rawsocket_server->shutdown();
        }

        // Allow the io service to stop when it runs out of work. Then
        // poll the io_service to finish processing all of the remaining
        // completion handlers that may need to still be executed as a
        // result of shutting down. It is then safe to stop the io service
        // without leaving any potentially unexecuted handlers.
        m_work.reset();
        m_ioService.poll();
        m_ioService.stop();
    }
}

void
WampRouter::termination_signal_handler(
        const boost::system::error_code& error_code, int signal_number)
{
    if (error_code == boost::asio::error::operation_aborted) {
        return;
    }

#if defined(SIGQUIT)
    if (signal_number == SIGINT || signal_number == SIGTERM || signal_number == SIGQUIT) {
#else
    if (signal_number == SIGINT || signal_number == SIGTERM) {
#endif
        shutdown();
    } else {
        // We should never encounter a case where we are invoked for a
        // signal for which we are not intentionally checking for. We
        // keep the assert for debugging purposes and re-register the
        // handler for production purposes so that we are still able to
        // terminate upon receipt of one of the registered signals.
        assert(0);
        m_termination_signals.async_wait(
                boost::bind(&WampRouter::termination_signal_handler, this, _1, _2));
    }
}
