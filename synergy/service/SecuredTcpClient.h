#ifndef SECUREDTCPCLIENT_H
#define SECUREDTCPCLIENT_H

#include "SecuredTcpSession.h"

class SecuredTcpClient
{
public:
    SecuredTcpClient(boost::asio::io_service& ioService, std::string hostname, std::string port);

    void connect();

    ssl::stream<tcp::socket>& stream();

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(SecuredTcpClient*)> connected;
    signal<void(SecuredTcpClient*)> connectFailed;

    std::string address() const;

private:
    void onResolveFinished(errorCode ec, tcp::resolver::iterator result);
    void onConnectFinished(errorCode ec);

private:
    boost::asio::io_service& m_ioService;
    boost::asio::ssl::context m_sslContext;
    SecuredTcpSession m_session;
    tcp::resolver m_resolver;
    std::string m_address;
    std::string m_port;
};

#endif // SECUREDTCPCLIENT_H
