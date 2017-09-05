#ifndef TCPSESSION_H
#define TCPSESSION_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/beast/core.hpp>
#include <boost/signals2.hpp>

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
typedef boost::system::error_code errorCode;

class SecuredTcpSession
{
public:
    SecuredTcpSession(boost::asio::io_service& ioService);
    virtual ~SecuredTcpSession();

    std::string hostname() const;
    void setHostname(const std::string &hostname);
    std::string port() const;
    void setPort(const std::string &port);
    ssl::stream<tcp::socket>& stream();

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(SecuredTcpSession*)> connected;
    signal<void(SecuredTcpSession*)> connectFailed;

    void connect();
    bool checkError(errorCode ec);

private:
    void onResolveFinished(errorCode ec, tcp::resolver::iterator result);
    void onConnectFinished(errorCode ec);
    void onSslHandshakeFinished(errorCode ec);

private:
    boost::asio::io_service& m_ioService;
    boost::asio::ssl::context m_sslContext;
    ssl::stream<tcp::socket> m_stream;
    tcp::resolver m_resolver;
    std::string m_hostname;
    std::string m_port;
};

#endif // TCPSESSION_H
