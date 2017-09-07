#include "WebsocketSession.h"

#include <synergy/service/Logs.h>
#include <boost/asio/connect.hpp>

static const long kReconnectDelaySec = 3;

WebsocketSession::WebsocketSession(boost::asio::io_service &ioService,
        const std::string& hostname,
        const std::string& port) :
    m_reconnectTimer(ioService),
    m_tcpClient(std::make_unique<SecuredTcpClient>(ioService, hostname, port)),
    m_websocket(std::make_unique<websocket::stream<ssl::stream<tcp::socket>&>>(m_tcpClient->stream())),
    m_target(),
    m_connected(false),
    m_ioService(ioService),
    m_hostname(hostname),
    m_port(port)
{
}

void
WebsocketSession::connect(const std::string target)
{
    m_target = target;

    m_tcpClient->connected.connect(
        [this](SecuredTcpClient*) {
            onTcpClientConnected();
        },
        boost::signals2::at_front
    );

    m_tcpClient->connectFailed.connect(
        [this](SecuredTcpClient*) {
            onTcpClientConnectFailed();
        },
        boost::signals2::at_front
    );

    mainLog()->debug("connecting websocket");
    m_tcpClient->connect();
}

void
WebsocketSession::disconnect()
{
    m_websocket->async_close(websocket::close_code::normal,
        std::bind(
            &WebsocketSession::onDisconnectFinished,
            this,
            std::placeholders::_1)
    );
}

void
WebsocketSession::reconnect()
{
    mainLog()->debug("retrying websocket connection in {}s", kReconnectDelaySec);

    if (m_connected) {
        mainLog()->debug("closing existing open connection");
        m_websocket->close(websocket::close_code::normal);
        m_connected = false;
    }

    m_reconnectTimer.expires_from_now(boost::posix_time::seconds(kReconnectDelaySec));

    m_reconnectTimer.async_wait([this](const boost::system::error_code&) {
        mainLog()->debug("retrying websocket connection now");

        m_tcpClient.reset(new SecuredTcpClient(m_ioService, m_hostname, m_port));
        m_websocket.reset(new websocket::stream<ssl::stream<tcp::socket>&>(m_tcpClient->stream()));

        connect(m_target);
    });
}

void
WebsocketSession::write(std::string& message)
{
    if (m_connected) {
        m_websocket->async_write(
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
WebsocketSession::onTcpClientConnected()
{
    // websocket handshake
    m_websocket->async_handshake_ex(
        m_tcpClient->address().c_str(),
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
WebsocketSession::onTcpClientConnectFailed()
{
    if (m_tcpClient) {
        m_tcpClient.reset();
        mainLog()->debug("websocket connect failed");
        reconnect();
    }
}

void
WebsocketSession::onWebsocketHandshakeFinished(errorCode ec)
{
    if (ec) {
        mainLog()->debug("websocket handshake error: {}", ec.message());
        reconnect();
        return;
    }

    m_connected = true;
    connected();
    mainLog()->debug("websocket connected");

    m_websocket->async_read(
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
    if (ec) {
        mainLog()->debug("websocket read error: {}", ec.message());
        return;
    }

    std::ostringstream stream;
    stream << boost::beast::buffers(m_readBuffer.data());
    std::string message =  stream.str();
    messageReceived(std::move(message));

    m_readBuffer = boost::beast::multi_buffer();

    // keep reading
    m_websocket->async_read(
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
    if (ec) {
        mainLog()->debug("websocket write error: {}", ec.message());
    }
}

void
WebsocketSession::onDisconnectFinished(errorCode ec)
{
    if (ec) {
        mainLog()->debug("websocket disconnect error: {}", ec.message());
    }

    m_connected = false;
    disconnected();
    mainLog()->debug("websocket disconnected");
}
