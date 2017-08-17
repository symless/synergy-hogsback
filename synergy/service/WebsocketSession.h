#ifndef WEBSOCKETSESSION_H
#define WEBSOCKETSESSION_H

#include <boost/asio/io_service.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace websocket = boost::beast::websocket;

class WebsocketSession
{
public:
    WebsocketSession(boost::asio::io_service& ioService);

    void connect();
    void disconnect();
    void write(std::string& message);

private:
    typedef boost::system::error_code errorCode;
    void loadCertificate(boost::asio::ssl::context& ctx);
    void checkError(errorCode ec);
    void onResolveFinished(errorCode ec,
                        tcp::resolver::iterator result);
    void onConnectFinished(errorCode ec);
    void onDisconnectFinished(errorCode ec);
    void onSslHandshakeFinished(errorCode ec);
    void onWebsocketHandshakeFinished(errorCode ec);
    void onReadFinished(errorCode ec);
    void onWriteFinished(errorCode ec);

private:
    boost::asio::io_service& m_ioService;
    boost::asio::ssl::context m_sslContext;
    websocket::stream<tcp::socket> m_websocket;
    tcp::resolver m_resolver;
    boost::beast::multi_buffer m_readBuffer;
    bool m_connected;
};

#endif // WEBSOCKETSESSION_H
