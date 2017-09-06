#include "HttpSession.h"

#include <boost/asio/connect.hpp>

static const int kHttpVersion = 11;

HttpSession::HttpSession(boost::asio::io_service& ioService, std::string hostname, std::string port) :
    m_tcpClient(ioService, hostname, port)
{
}

void HttpSession::get(const std::string &target)
{
    setupRequest(http::verb::get, target);

    connect();
}

void HttpSession::post(const std::string &target, const std::string &body)
{
    setupRequest(http::verb::post, target, body);

    connect();
}

void HttpSession::connect()
{
    m_tcpClient.connected.connect(
        [this](SecuredTcpClient*) {
            onTcpClientConnected();
        },
        boost::signals2::at_front
    );

    m_tcpClient.connect();
}

void HttpSession::onTcpClientConnected()
{
    http::async_write(m_tcpClient.stream(), m_request,
        std::bind(
            &HttpSession::onWriteFinished,
            this,
            std::placeholders::_1));
}

void HttpSession::setupRequest(http::verb method, const std::string &target, const std::string &body)
{
    m_request.version = kHttpVersion;
    m_request.method(method);
    m_request.target(target);
    //m_request.set(http::field::host, m_session.hostname().c_str());

    if (!body.empty()) {
        m_request.set(http::field::content_type, "application/json");
        m_request.body = body.c_str();
    }

    // TODO: add user token
}

void HttpSession::onWriteFinished(errorCode ec)
{
    //m_tcpClient.checkError(ec);

    // TODO: do we care if the request has been received?
}
