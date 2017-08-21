#include "SecuredTcpSession.h"

#include <boost/asio/connect.hpp>
#include <iostream>

SecuredTcpSession::SecuredTcpSession(boost::asio::io_service& ioService) :
    m_ioService(ioService),
    m_sslContext(ssl::context::tlsv12_client),
    m_stream(ioService, m_sslContext),
    m_resolver(ioService),
    m_hostname(),
    m_port()
{

}

SecuredTcpSession::~SecuredTcpSession()
{

}

std::string SecuredTcpSession::hostname() const
{
    return m_hostname;
}

void SecuredTcpSession::setHostname(const std::string &hostname)
{
    m_hostname = hostname;
}

std::string SecuredTcpSession::port() const
{
    return m_port;
}

void SecuredTcpSession::setPort(const std::string &port)
{
    m_port = port;
}

void SecuredTcpSession::connect()
{
    m_resolver.async_resolve(
        {hostname(), port()},
        std::bind(
            &SecuredTcpSession::onResolveFinished,
            this,
            std::placeholders::_1,
            std::placeholders::_2));
}

bool SecuredTcpSession::checkError(errorCode ec)
{
    if (ec) {
       // TODO: use logging subsystem
       std::cout << ec << std::endl;
    }

    return ec;
}

void SecuredTcpSession::onResolveFinished(errorCode ec, tcp::resolver::iterator result)
{
    if (checkError(ec)) {
        connectFailed();
        return;
    }

    boost::asio::async_connect(
        m_stream.next_layer(),
        result,
        std::bind(
            &SecuredTcpSession::onConnectFinished,
            this,
            std::placeholders::_1));
}

void SecuredTcpSession::onConnectFinished(errorCode ec)
{
    if (checkError(ec)) {
        connectFailed();
        return;
    }

    // SSL handshake
    m_stream.async_handshake(
        ssl::stream_base::client,
        std::bind(
            &SecuredTcpSession::onSslHandshakeFinished,
            this,
            std::placeholders::_1));
}

void SecuredTcpSession::onSslHandshakeFinished(errorCode ec)
{
    if (checkError(ec)) {
        connectFailed();
        return;
    }

    connected();
}

ssl::stream<tcp::socket>& SecuredTcpSession::stream()
{
    return m_stream;
}
