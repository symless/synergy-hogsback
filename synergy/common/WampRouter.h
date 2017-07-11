#ifndef WAMPROUTER_H
#define WAMPROUTER_H

#include <boost/asio.hpp>
#include <bonefish/serialization/wamp_serializers.hpp>
#include <bonefish/serialization/json_serializer.hpp>
#include <bonefish/serialization/msgpack_serializer.hpp>
#include <bonefish/router/wamp_router.hpp>
#include <bonefish/router/wamp_routers.hpp>
#include <bonefish/rawsocket/rawsocket_server.hpp>
#include <bonefish/rawsocket/tcp_listener.hpp>
#include <bonefish/trace/trace.hpp>
#include <bonefish/websocket/websocket_server.hpp>

class WampRouter
{
public:
    WampRouter(std::string ip, int port);
    ~WampRouter();

    void run();
    void shutdown();
    boost::asio::io_service& io_service();

private:
    void shutdown_handler();
    void termination_signal_handler(
            const boost::system::error_code& error_code, int signal_number);

private:
    boost::asio::io_service m_io_service;
    std::shared_ptr<boost::asio::io_service::work> m_work;
    boost::asio::signal_set m_termination_signals;

    std::shared_ptr<bonefish::wamp_routers> m_routers;
    std::shared_ptr<bonefish::wamp_serializers> m_serializers;
    std::shared_ptr<bonefish::rawsocket_server> m_rawsocket_server;
    std::shared_ptr<bonefish::websocket_server> m_websocket_server;

    std::uint16_t m_websocket_port;
};

#endif // WAMPROUTER_H
