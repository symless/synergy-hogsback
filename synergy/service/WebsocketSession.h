#ifndef WEBSOCKETSESSION_H
#define WEBSOCKETSESSION_H

#include "synergy/service/SecuredTcpClient.h"

#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>
#include <map>

namespace websocket = boost::beast::websocket;
namespace ssl = boost::asio::ssl;

class WebsocketSession final
{
public:
    WebsocketSession(boost::asio::io_service& ioService, const std::string& hostname, const std::string& port);

    void connect(const std::string target);
    void disconnect();
    void reconnect();
    void write(std::string& message);
    void addHeader(std::string headerName, std::string headerContent);
    bool isConnected();

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void()> connected;
    signal<void()> disconnected;
    signal<void(std::string)> messageReceived;

private:
    void onSessionConnected();

    void onWebsocketHandshakeFinished(errorCode ec);
    void onReadFinished(errorCode ec);
    void onWriteFinished(errorCode ec);
    void onDisconnectFinished(errorCode ec);


private:
    boost::beast::multi_buffer m_readBuffer;
    boost::asio::deadline_timer m_reconnectTimer;
    SecuredTcpClient m_tcpClient;
    websocket::stream<ssl::stream<tcp::socket>&> m_websocket;
    std::string m_target;
    std::map<std::string, std::string> m_headers;
    bool m_connected;
};

#endif // WEBSOCKETSESSION_H
