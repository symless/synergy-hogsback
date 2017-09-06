#include "SecuredTcpSession.h"

#include <iostream>

SecuredTcpSession::SecuredTcpSession(boost::asio::io_service& ioService, ssl::context &context) :
    m_ioService(ioService),
    m_sslContext(context),
    m_stream(ioService, m_sslContext)
{
}

SecuredTcpSession::~SecuredTcpSession()
{
}

void SecuredTcpSession::startSslHandshake(bool serverMode)
{
    if (serverMode) {
        // SSL handshake
        m_stream.async_handshake(
            ssl::stream_base::server,
            std::bind(
                &SecuredTcpSession::onSslHandshakeFinished,
                this,
                std::placeholders::_1));
    }
    else {
        // SSL handshake
        m_stream.async_handshake(
            ssl::stream_base::client,
            std::bind(
                &SecuredTcpSession::onSslHandshakeFinished,
                this,
                std::placeholders::_1));
    }

}

void SecuredTcpSession::onSslHandshakeFinished(errorCode ec)
{
    if (ec) {
        std::string error = ec.message();
        connectFailed(this);
        return;
    }

    connected(this);
}

ssl::stream<tcp::socket>& SecuredTcpSession::stream()
{
    return m_stream;
}
