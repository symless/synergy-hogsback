#include "WebsocketSession.h"

#include <boost/asio/connect.hpp>

static const long kReconnectDelaySec = 3;

WebsocketSession::WebsocketSession(
        boost::asio::io_service &ioService,
        std::string hostname,
        std::string port) :
    m_reconnectTimer(ioService),
    m_session(ioService),
    m_websocket(m_session.stream()),
    m_connected(false)
{
    m_session.setHostname(hostname);
    m_session.setPort(port);
}

void
WebsocketSession::connect()
{
    m_session.connected.connect(
        [this]() {
            onSessionConnected();
        },
        boost::signals2::at_front
    );

    m_session.connect();
}

void
WebsocketSession::disconnect()
{
    m_websocket.async_close(websocket::close_code::normal,
        std::bind(
            &WebsocketSession::onDisconnectFinished,
            this,
            std::placeholders::_1)
    );
}

void
WebsocketSession::reconnect()
{
    if (m_connected) {
        m_websocket.close(websocket::close_code::normal);
        m_connected = false;
    }

    m_reconnectTimer.cancel();
    m_reconnectTimer.expires_from_now(boost::posix_time::seconds(kReconnectDelaySec));

    m_reconnectTimer.async_wait([this](const boost::system::error_code&) {
        connect();
    });

}

void
WebsocketSession::write(std::string& message)
{
    if (m_connected) {
        m_websocket.async_write(
            boost::asio::buffer(message),
            std::bind(
                &WebsocketSession::onWriteFinished,
                this,
                std::placeholders::_1)
        );
    }
}

void
WebsocketSession::onSessionConnected()
{

    // TODO: get user ID as the unique pubsub channel
    // TODO: put user token into websocket request header
    const char* fakeChannel = "/pubsub/auth/1";

    // websocket handshake
    m_websocket.async_handshake_ex(
        m_session.hostname().c_str(),
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
WebsocketSession::onWebsocketHandshakeFinished(errorCode ec)
{
    if (m_session.checkError(ec)) {
        reconnect();
        return;
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
WebsocketSession::onReadFinished(errorCode ec)
{
    if (m_session.checkError(ec)) {
        return;
    }

    std::ostringstream stream;
    stream << boost::beast::buffers(m_readBuffer.data());
    std::string message =  stream.str();
    messageReceived(std::move(message));

    // TODO: pass message to process manager
    m_readBuffer.consume(m_readBuffer.size());

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
WebsocketSession::onWriteFinished(errorCode ec)
{
    m_session.checkError(ec);
}

void
WebsocketSession::onDisconnectFinished(errorCode ec)
{
    m_session.checkError(ec);

    m_connected = false;
    disconnected();
}
