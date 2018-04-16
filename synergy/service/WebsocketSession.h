#ifndef WEBSOCKETSESSION_H
#define WEBSOCKETSESSION_H

#include "synergy/service/SecuredTcpClient.h"
#include <synergy/service/WebsocketError.h>

#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>
#include <map>
#include <memory>

class WebsocketSession final
{
public:
    using ErrorCode = SecuredTcpClient::ErrorCode;
    using Stream    = boost::beast::websocket::stream<SecuredTcpClient::Stream&>;

    WebsocketSession(boost::asio::io_service& ioService,
                     const std::string& hostname, const std::string& port);
    ~WebsocketSession();

    void connect(const std::string target);
    void reconnect(bool now = false);
    void write(std::string& message);
    void addHeader(std::string headerName, std::string headerContent);
    bool isConnected() const;
    void shutdown() noexcept;

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void()> connected;
    signal<void()> disconnected;
    signal<void(WebsocketError error)> connectionError;
    signal<void(std::string)> messageReceived;

private:
    void onTcpClientConnected();
    void onTcpClientConnectFailed();
    void onWebsocketHandshakeFinished(ErrorCode ec);
    void onReadFinished(ErrorCode ec);
    void onWriteFinished(ErrorCode ec);
    void setTcpKeepAliveTimeout();
    void handleConnectError(bool reconnect, WebsocketError error = WebsocketError::kUnknown);
    void initSockets();

private:
    boost::beast::multi_buffer m_readBuffer;
    boost::asio::deadline_timer m_reconnectTimer;
    std::unique_ptr<SecuredTcpClient> m_tcpClient;
    std::unique_ptr<Stream> m_websocket;
    boost::beast::websocket::response_type m_res;
    std::string m_target;
    std::map<std::string, std::string> m_headers;
    bool m_connected;
    boost::asio::io_service& m_ioService;
    std::string m_hostname;
    std::string m_port;
    bool m_connecting = false;
};

#endif // WEBSOCKETSESSION_H
