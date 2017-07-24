#ifndef WAMPROUTER_H
#define WAMPROUTER_H

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

namespace bonefish {
    class rawsocket_server;
    class wamp_routers;
    class wamp_serializers;
    class websocket_server;
}

class WampRouter
{
public:
    WampRouter(boost::asio::io_service& ioService, std::string ip, int port);
    ~WampRouter();

    void start();
    void stop();

    boost::signals2::signal<void()> ready;

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<bonefish::wamp_routers> m_routers;
    std::shared_ptr<bonefish::wamp_serializers> m_serializers;
    std::shared_ptr<bonefish::rawsocket_server> m_rawsocketServer;
    std::shared_ptr<bonefish::websocket_server> m_websocketServer;
    std::uint16_t m_websocketPort;
};

#endif // WAMPROUTER_H
