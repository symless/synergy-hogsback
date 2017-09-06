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

void SecuredTcpSession::startSslHandshake()
{
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
    if (ec) {
        connectFailed(this);
        return;
    }

    connected(this);
}

ssl::stream<tcp::socket>& SecuredTcpSession::stream()
{
    return m_stream;
}
