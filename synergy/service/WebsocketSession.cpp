#include "WebsocketSession.h"

#include <boost/asio/connect.hpp>

static const char* kServerHostname = "192.168.3.93";
static const char* kServerPort = "80";

WebsocketSession::WebsocketSession(boost::asio::io_service &ioService) :
    m_ioService(ioService),
    m_sslContext(ssl::context::sslv3_client),
    m_websocket(ioService),
    m_resolver(ioService),
    m_connected(false)
{
    loadCertificate(m_sslContext);
}

void WebsocketSession::connect()
{
    m_resolver.async_resolve(
        {kServerHostname, kServerPort},
        std::bind(
            &WebsocketSession::onResolveFinished,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2)
    );
}

void WebsocketSession::disconnect()
{

    m_websocket.async_close(websocket::close_code::normal,
        std::bind(
            &WebsocketSession::onDisconnectFinished,
            shared_from_this(),
            std::placeholders::_1)
                            );
}

void WebsocketSession::write(std::string& message)
{
    m_websocket.async_write(
        boost::asio::buffer(message),
        std::bind(
            &WebsocketSession::onWriteFinished,
            shared_from_this(),
            std::placeholders::_1)
    );
}

void
WebsocketSession::loadCertificate(ssl::context &ctx)
{
    // TODO: server certificate management
}

void
WebsocketSession::checkError(errorCode ec)
{
    if (ec) {
        // TODO: handle failure
       std::cout << ec << std::endl;
       return;
    }
}

void
WebsocketSession::onResolveFinished(errorCode ec,
                                  tcp::resolver::iterator result)
{
    checkError(ec);

    boost::asio::async_connect(
        m_websocket.next_layer(),
        result,
        std::bind(
            &WebsocketSession::onConnectFinished,
            shared_from_this(),
            std::placeholders::_1)
    );
}

void
WebsocketSession::onConnectFinished(errorCode ec)
{
    checkError(ec);

//    // SSL handshake
    // TODO: use SSL
//    m_websocket.next_layer().async_handshake(
//        ssl::stream_base::client,
//        std::bind(
//            &WebsocketSession::onSslHandshakeFinished,
//            shared_from_this(),
//            std::placeholders::_1)
//    );

    onSslHandshakeFinished(ec);

}

void
WebsocketSession::onDisconnectFinished(errorCode ec)
{
    // TODO: closed notify whoever is interested
}

void
WebsocketSession::onSslHandshakeFinished(errorCode ec)
{
    checkError(ec);

    // TODO: get user ID as the unique pubsub channel
    // TODO: put user token into websocket request header
    const char* fakeChannel = "/pubsub/auth/1";

    // websocket handshake
    m_websocket.async_handshake(
        kServerHostname,
        fakeChannel,
        std::bind(
            &WebsocketSession::onWebsocketHandshakeFinished,
            shared_from_this(),
            std::placeholders::_1)
    );
}

void
WebsocketSession::onWebsocketHandshakeFinished(WebsocketSession::errorCode ec)
{
    m_connected = true;

    m_websocket.async_read(
        m_readBuffer,
        std::bind(
            &WebsocketSession::onReadFinished,
            shared_from_this(),
            std::placeholders::_1)
    );
}

void
WebsocketSession::onReadFinished(WebsocketSession::errorCode ec)
{
    checkError(ec);

    // TODO: pass message to process manager

    // keep reading
    m_websocket.async_read(
        m_readBuffer,
        std::bind(
            &WebsocketSession::onReadFinished,
            shared_from_this(),
            std::placeholders::_1)
    );
}

void
WebsocketSession::onWriteFinished(WebsocketSession::errorCode ec)
{
    checkError(ec);
}
