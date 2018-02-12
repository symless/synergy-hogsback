#include "WebsocketSession.h"

#include <synergy/service/ServiceLogs.h>
#include <boost/asio/connect.hpp>
#include <fmt/ostream.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <Windows.h>
#include <MSTcpIP.h>
#else
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#endif

// randomly reconnect between this min and max.
static const long kMinReconnectDelaySec = 5;
static const long kMaxReconnectDelaySec = 30;

// user should wait no more than 5 seconds before realizing the connection is down.
static const unsigned int kTcpKeepAliveIdleSec = 5;

// once we've had a failure, try frequently to fail quickly (so the user isn't
// waiting around for ages wondering what's going on.
static const unsigned int kTcpKeepAliveIntervalSec = 1;

// claim the connection is down after this number of keep alive retries.
// BUG: this is hard coded to 10 seconds on windows (we can't change it).
static const unsigned int kTcpKeepAliveCount = 2;

WebsocketSession::WebsocketSession(boost::asio::io_service &ioService,
        const std::string& hostname,
        const std::string& port) :
    m_reconnectTimer(ioService),
    m_target(),
    m_connected(false),
    m_ioService(ioService),
    m_hostname(hostname),
    m_port(port)
{
    srand (time(NULL));
}

WebsocketSession::~WebsocketSession()
{
    shutdown();
}

void
WebsocketSession::initSockets()
{
    m_tcpClient.reset(new SecuredTcpClient(m_ioService, m_hostname, m_port));
    m_websocket.reset(new websocket::stream<ssl::stream<tcp::socket>&>(m_tcpClient->stream()));
}

void
WebsocketSession::connect(const std::string target)
{
    if (m_connecting) {
        serviceLog()->warn("already connecting websocket");
    }

    initSockets();

    m_connecting = true;
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

    serviceLog()->debug("connecting websocket: {}:{}", m_hostname, m_port);
    m_tcpClient->connect();
}

void
WebsocketSession::reconnect(bool now)
{
    int random = rand() % (kMaxReconnectDelaySec - kMinReconnectDelaySec + 1);
    int reconnectDelay = kMinReconnectDelaySec + random;

    if (now) {
        // HACK: it's a bit ugly...
        reconnectDelay = 0;
    }
    else {
        serviceLog()->debug("retrying websocket connection in {}s", reconnectDelay);
    }

    shutdown();

    m_reconnectTimer.cancel();
    m_reconnectTimer.expires_from_now(boost::posix_time::seconds(reconnectDelay));
    m_reconnectTimer.async_wait([this](const boost::system::error_code& ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }
        serviceLog()->debug("retrying websocket connection now");
        initSockets();
        connect(m_target);
    });
}

void
WebsocketSession::handleConnectError(bool reconnect_, WebsocketError error)
{
    m_connecting = false;

    connectionError(error);

    if (reconnect_) {
        reconnect();
    }
    else {
        serviceLog()->warn("abandoning websocket connection attempt");
    }
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

bool WebsocketSession::isConnected() const
{
    return m_connected;
}

void
WebsocketSession::onTcpClientConnected()
{
    setTcpKeepAliveTimeout();

    // websocket handshake
    m_websocket->async_handshake_ex(
        m_res,
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

        serviceLog()->error("websocket connect failed");
        handleConnectError(true, WebsocketError::kConnection);
    }
}

void
WebsocketSession::onWebsocketHandshakeFinished(errorCode ec)
{
    if (ec) {
        std::string res_failed_reason = m_res["X-SCS-Reason"].to_string();
        if (m_res.result() == boost::beast::http::status::forbidden && !res_failed_reason.empty()) {
            serviceLog()->error("websocket connection failed: {}", res_failed_reason);
            handleConnectError(false, WebsocketError::kAuth);

            return;
        }

        serviceLog()->error("websocket handshake error {}: {}", ec.value(), ec.message());
        serviceLog()->error("websocket handshake response {}: {}", (int)m_res.result(), m_res.result());
        handleConnectError(true, WebsocketError::kConnection);

        return;
    }

    m_connecting = false;
    m_connected = true;
    connected();
    serviceLog()->debug("websocket connected");

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
        if (ec != boost::asio::error::operation_aborted) {
            serviceLog()->error("websocket read error {}: {}", ec.value(), ec.message());
            handleConnectError(true, WebsocketError::kRead);
        }

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
        serviceLog()->error("websocket write error {}: {}", ec.value(), ec.message());
    }
}

void WebsocketSession::shutdown() noexcept
{
    if (m_connected) {
        serviceLog()->debug("closing existing websocket connection");
        errorCode ec;
        m_reconnectTimer.cancel();
        m_websocket->close(websocket::close_code::normal, ec);
        m_connecting = false;
        m_connected = false;
    }
}

void WebsocketSession::setTcpKeepAliveTimeout()
{
    auto& socket = m_tcpClient->stream().next_layer();

#ifdef _WIN32
    struct tcp_keepalive options;
    options.onoff = 1;
    options.keepalivetime = kTcpKeepAliveIdleSec * 1000;
    options.keepaliveinterval = kTcpKeepAliveIntervalSec * 1000;

    BOOL on = true;
    SOCKET native = socket.native();
    DWORD bytesReturned;

    int keepaliveResult = setsockopt(native, SOL_SOCKET, SO_KEEPALIVE, (const char *) &on, sizeof(on));

    if (keepaliveResult) {
        serviceLog()->error("enable keepalive failed");
        return;
    }

    int iotclResult = WSAIoctl(native, SIO_KEEPALIVE_VALS, (LPVOID) & options, (DWORD) sizeof(options), NULL, 0,
                (LPDWORD) & bytesReturned, NULL, NULL);

    if (iotclResult) {
        serviceLog()->error("set keepalive timeout and interval failed");
        return;
    }
#else
    int nativeSocket= socket.native();

    int on = 1;
    int enableResult = 0;
    enableResult = setsockopt(nativeSocket, SOL_SOCKET,  SO_KEEPALIVE, &on, sizeof(int));
    if (enableResult == -1) {
        serviceLog()->error("enable keepalive failed: {}", std::string(strerror(errno)));
        return;
    }

#ifdef __APPLE__
    int keepaliveResult = 0;
    int keepaliveTimeout = kTcpKeepAliveIdleSec;
    keepaliveResult = setsockopt(nativeSocket, IPPROTO_TCP, TCP_KEEPALIVE, &keepaliveTimeout, sizeof(int));
    if (keepaliveResult == -1) {
        serviceLog()->error("set keepalive timeout failed: {}", std::string(strerror(errno)));
        return;
    }
#else
    int keepaliveResult = 0;
    int keepaliveTimeout = kTcpKeepAliveIdleSec;
    keepaliveResult = setsockopt(nativeSocket, SOL_TCP, TCP_KEEPIDLE, &keepaliveTimeout, sizeof(int));
    if (keepaliveResult == -1) {
        serviceLog()->error("set keepalive timeout failed: {}", std::string(strerror(errno)));
        return;
    }
#endif
    int timeoutInterval = kTcpKeepAliveIntervalSec;
    int intvlResult = setsockopt(nativeSocket, IPPROTO_TCP, TCP_KEEPINTVL, &timeoutInterval, sizeof(int));
    if (intvlResult == -1) {
        serviceLog()->error("set keepalive interval failed: {}", std::string(strerror(errno)));
        return;
    }

    int timeoutCount = kTcpKeepAliveCount;
    int countResult = setsockopt(nativeSocket, IPPROTO_TCP, TCP_KEEPCNT, &timeoutCount, sizeof(int));
    if (countResult == -1) {
        serviceLog()->error("set keepalive timeout count failed: {}", std::string(strerror(errno)));
        return;
    }
#endif
}
