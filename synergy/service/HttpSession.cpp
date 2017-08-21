#include "HttpSession.h"

#include <boost/asio/connect.hpp>

static const int kHttpVersion = 11;

HttpSession::HttpSession(boost::asio::io_service& ioService, std::string hostname, std::string port) :
    m_session(ioService),
    m_stream(m_session.stream())
{
    m_session.setHostname(hostname);
    m_session.setPort(port);
}

void HttpSession::get(const std::string &target)
{
    setupRequest(http::verb::get, target);

    m_session.connected.connect(
        [this]() {
            onSessionConnected();
        },
        boost::signals2::at_front
    );

    m_session.connect();
}

void HttpSession::onSessionConnected()
{
    http::async_write(m_stream, m_request,
        std::bind(
            &HttpSession::onWriteFinished,
            this,
            std::placeholders::_1));
}

void HttpSession::setupRequest(http::verb method, const std::string &target)
{
    m_request.version = kHttpVersion;
    m_request.method(method);
    m_request.target(target);
    m_request.set(http::field::host, m_session.hostname().c_str());
    // TODO: add user token
}

void HttpSession::onWriteFinished(errorCode ec)
{
    m_session.checkError(ec);

    // TODO: do we care if the request has been received?
}
