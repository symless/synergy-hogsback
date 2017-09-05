#include "WebsocketSession.h"

#include <boost/asio/connect.hpp>

static const long kReconnectDelaySec = 3;

WebsocketSession::WebsocketSession(boost::asio::io_service &ioService,
        const std::string& hostname,
        const std::string& port) :
    m_reconnectTimer(ioService),
    m_session(ioService),
    m_websocket(m_session.stream()),
    m_target(),
    m_connected(false)
{
    m_session.setHostname(hostname);
    m_session.setPort(port);
}

void
WebsocketSession::connect(const std::string target)
{
    m_target = target;

    m_session.connected.connect(
        [this](SecuredTcpSession*) {
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
        connect(m_target);
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

void WebsocketSession::addHeader(std::string headerName, std::string headerContent)
{
    m_headers[headerName] = headerContent;
}

bool WebsocketSession::isConnected()
{
    return m_connected;
}

void
WebsocketSession::onSessionConnected()
{
    // websocket handshake
    m_websocket.async_handshake_ex(
        m_session.hostname().c_str(),
        m_target.c_str(),
        [this](boost::beast::websocket::request_type & req) {
            for (auto const& i : m_headers) {
                req.set(i.first, i.second);
            }
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
    connected();

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

    m_readBuffer = boost::beast::multi_buffer();

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
