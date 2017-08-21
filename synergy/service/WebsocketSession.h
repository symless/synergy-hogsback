#ifndef WEBSOCKETSESSION_H
#define WEBSOCKETSESSION_H

#include "synergy/service/SecuredTcpSession.h"

#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>

namespace websocket = boost::beast::websocket;
namespace ssl = boost::asio::ssl;

class WebsocketSession final
{
public:
    WebsocketSession(boost::asio::io_service& ioService, std::string hostname, std::string port);

    void connect();
    void disconnect();
    void reconnect();
    void write(std::string& message);

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void()> disconnected;
    signal<void(std::string const&)> messageReceived;

private:
    void onSessionConnected();

    void onWebsocketHandshakeFinished(errorCode ec);
    void onReadFinished(errorCode ec);
    void onWriteFinished(errorCode ec);
    void onDisconnectFinished(errorCode ec);


private:
    boost::beast::multi_buffer m_readBuffer;
    boost::asio::deadline_timer m_reconnectTimer;
    SecuredTcpSession m_session;
    websocket::stream<ssl::stream<tcp::socket>&> m_websocket;
    bool m_connected;
};

#endif // WEBSOCKETSESSION_H
