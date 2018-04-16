#ifndef SECUREDTCPCLIENT_H
#define SECUREDTCPCLIENT_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>
#include <synergy/service/ProxyClient.h>

class SecuredTcpClient
{
public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    using ErrorCode = boost::system::error_code;
    using Stream = boost::asio::ssl::stream
                    <ProxyClient<boost::asio::ip::tcp::socket>>;

    SecuredTcpClient(boost::asio::io_service& ioService,
                     std::string hostname, std::string port);

    void connect();
    Stream& stream();

    signal<void(SecuredTcpClient*)> connected;
    signal<void(SecuredTcpClient*)> connectFailed;

    std::string address() const;
    std::string port() const;

private:
    void onResolveFinished(ErrorCode ec,
                           boost::asio::ip::tcp::resolver::iterator result);
    void onConnectFinished(ErrorCode ec);
    void onSslHandshakeFinished(ErrorCode ec);

private:
    boost::asio::io_service& m_ioService;
    boost::asio::ssl::context m_sslContext;
    Stream m_stream;
    boost::asio::ip::tcp::resolver m_resolver;
    std::string m_address;
    std::string m_port;
    bool m_connected = false;
};

#endif // SECUREDTCPCLIENT_H
