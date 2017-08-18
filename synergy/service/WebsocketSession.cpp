#include "WebsocketSession.h"

#include <boost/asio/connect.hpp>

static const char* kServerHostname = "192.168.3.93";
static const char* kServerPort = "80";

WebsocketSession::WebsocketSession(boost::asio::io_service &ioService) :
    m_ioService(ioService),
    m_sslContext(ssl::context::sslv3_client),
    m_websocket(ioService),
    m_resolver(ioService),
    m_reconnectTimer(ioService),
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
            this,
            std::placeholders::_1,
            std::placeholders::_2)
    );
}

void WebsocketSession::disconnect()
{

    m_websocket.async_close(websocket::close_code::normal,
        std::bind(
            &WebsocketSession::onDisconnectFinished,
            this,
            std::placeholders::_1)
    );
}

void WebsocketSession::reconnect(long waitSec)
{
    if (m_connected) {
        m_websocket.close(websocket::close_code::normal);
        m_connected = false;
    }

    if (waitSec > 0) {
        m_reconnectTimer.cancel();
        m_reconnectTimer.expires_from_now(boost::posix_time::seconds(waitSec));

        m_reconnectTimer.async_wait([this](const boost::system::error_code&) {
            connect();
        });
    }
}

void WebsocketSession::write(std::string& message)
{
    m_websocket.async_write(
        boost::asio::buffer(message),
        std::bind(
            &WebsocketSession::onWriteFinished,
            this,
            std::placeholders::_1)
    );
}

void
WebsocketSession::loadCertificate(ssl::context &ctx)
{
    // TODO: server certificate management
}

bool
WebsocketSession::checkError(errorCode ec)
{
    if (ec) {
       // TODO: use logging subsystem
       std::cout << ec << std::endl;
    }

    return ec;
}

void
WebsocketSession::onResolveFinished(errorCode ec,
                                  tcp::resolver::iterator result)
{
    if (checkError(ec)) {
        reconnectRequired();
    }

    boost::asio::async_connect(
        m_websocket.next_layer(),
        result,
        std::bind(
            &WebsocketSession::onConnectFinished,
            this,
            std::placeholders::_1)
    );
}

void
WebsocketSession::onConnectFinished(errorCode ec)
{
    if (checkError(ec)) {
        reconnectRequired();
    }

//    // SSL handshake
    // TODO: use SSL
//    m_websocket.next_layer().async_handshake(
//        ssl::stream_base::client,
//        std::bind(
//            &WebsocketSession::onSslHandshakeFinished,
//            this,
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
    if (checkError(ec)) {
        reconnectRequired();
    }

    // TODO: get user ID as the unique pubsub channel
    // TODO: put user token into websocket request header
    const char* fakeChannel = "/pubsub/auth/1";

    // websocket handshake
    m_websocket.async_handshake_ex(
        kServerHostname,
        fakeChannel,
        [](boost::beast::websocket::request_type & req) {
            req.set("X-Auth-Token", "Test-Token");
        },
        std::bind(
            &WebsocketSession::onWebsocketHandshakeFinished,
            this,
            std::placeholders::_1)
    );
}

void
WebsocketSession::onWebsocketHandshakeFinished(WebsocketSession::errorCode ec)
{
    if (checkError(ec)) {
        reconnectRequired();
    }

    m_connected = true;

    m_websocket.async_read(
        m_readBuffer,
        std::bind(
            &WebsocketSession::onReadFinished,
            this,
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
            this,
            std::placeholders::_1)
    );
}

void
WebsocketSession::onWriteFinished(WebsocketSession::errorCode ec)
{
    checkError(ec);
}
