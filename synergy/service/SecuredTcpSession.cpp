#include "SecuredTcpSession.h"

#include <synergy/service/ServiceLogs.h>
#include <iostream>

SecuredTcpSession::SecuredTcpSession(boost::asio::io_service& ioService, ssl::context &context) :
    m_ioService(ioService),
    m_stream(ioService, context)
{
}

SecuredTcpSession::~SecuredTcpSession()
{
    boost::system::error_code ec;
    m_stream.shutdown(ec);
}

ssl::stream<tcp::socket>& SecuredTcpSession::stream()
{
    return m_stream;
}
