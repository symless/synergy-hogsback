#ifndef WEBSOCKETSESSION_H
#define WEBSOCKETSESSION_H

#include "synergy/service/SecuredTcpClient.h"

#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>
#include <map>
#include <memory>

namespace websocket = boost::beast::websocket;
namespace ssl = boost::asio::ssl;

class WebsocketSession final
{
public:
    WebsocketSession(boost::asio::io_service& ioService, const std::string& hostname, const std::string& port);
    ~WebsocketSession();

    void connect(const std::string target);
    void reconnect(bool now = false);
    void write(std::string& message);
    void addHeader(std::string headerName, std::string headerContent);
    bool isConnected() const;
    bool fatalConnectionError() const;

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void()> connected;
    signal<void()> disconnected;
    signal<void()> connectionError;
    signal<void(std::string)> messageReceived;

private:
    void onTcpClientConnected();
    void onTcpClientConnectFailed();
    void onWebsocketHandshakeFinished(errorCode ec);
    void onReadFinished(errorCode ec);
    void onWriteFinished(errorCode ec);

    void close() noexcept;
    void setTcpKeepAliveTimeout();
    void handleConnectError(bool reconnect, bool isFatal);
    void initSockets();

private:
    boost::beast::multi_buffer m_readBuffer;
    boost::asio::deadline_timer m_reconnectTimer;
    std::unique_ptr<SecuredTcpClient> m_tcpClient;
    std::unique_ptr<websocket::stream<ssl::stream<tcp::socket>&>> m_websocket;
    std::string m_target;
    std::map<std::string, std::string> m_headers;
    bool m_connected;
    boost::asio::io_service& m_ioService;
    std::string m_hostname;
    std::string m_port;
    bool m_connecting = false;
    bool m_fatalConnectionError = false;
};

#endif // WEBSOCKETSESSION_H
