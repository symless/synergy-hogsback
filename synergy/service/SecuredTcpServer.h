#ifndef SECUREDTCPSERVER_H
#define SECUREDTCPSERVER_H

#include "SecuredTcpSession.h"

#include <boost/signals2.hpp>

class SecuredTcpServer
{
public:
    SecuredTcpServer(boost::asio::io_service& ioService);

    void start();

    std::string address() const;
    void setAddress(const std::string &address);
    std::string port() const;
    void setPort(const std::string &port);
    void loadRawCetificate(const std::string& cetificate, const std::string& privateKey, const std::string &dh);

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;
    signal<void(SecuredTcpServer*)> connectFailed;

private:
    void accept();
    void onAccept(SecuredTcpSession *session, boost::system::error_code ec);
    void onSslHandshakeFinished(SecuredTcpSession *session, errorCode ec);

private:
    boost::asio::io_service& m_ioService;
    boost::asio::ssl::context m_sslContext;
    tcp::acceptor m_acceptor;
    std::string m_address;
    std::string m_port;
    std::vector<std::unique_ptr<SecuredTcpSession>> m_sessions;
};

#endif // SECUREDTCPSERVER_H
