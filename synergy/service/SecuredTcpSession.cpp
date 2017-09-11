#include "SecuredTcpSession.h"

#include <synergy/service/Logs.h>
#include <iostream>

SecuredTcpSession::SecuredTcpSession(boost::asio::io_service& ioService, ssl::context &context) :
    m_ioService(ioService),
    m_sslContext(context),
    m_stream(ioService, m_sslContext)
{
}

SecuredTcpSession::~SecuredTcpSession()
{
    try {
        m_stream.lowest_layer().shutdown(boost::asio::socket_base::shutdown_both);
    }
    catch (const std::exception& ex) {
        mainLog()->error("unable to shutdown tcp session: {}", ex.what());
    }
}

ssl::stream<tcp::socket>& SecuredTcpSession::stream()
{
    return m_stream;
}
