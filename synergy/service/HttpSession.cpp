#include "HttpSession.h"

#include <synergy/service/Logs.h>
#include <boost/asio/connect.hpp>

static const int kHttpVersion = 11;

HttpSession::HttpSession(boost::asio::io_service& ioService, std::string hostname, std::string port) :
    m_tcpClient(ioService, hostname, port),
    m_readBuffer(2048)
{
}

void HttpSession::addHeader(std::string headerName, std::string headerContent)
{
    m_headers[headerName] = headerContent;
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

    m_tcpClient.connectFailed.connect(
        [this](SecuredTcpClient*){
        requestFailed(this, "TCP connection failed");
    });

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
    m_request.target(std::move(target));
    m_request.set(http::field::host, m_tcpClient.address().c_str());
    m_request.set(http::field::connection, "keep-alive");

    if (!body.empty()) {
        m_request.set(http::field::content_type, "application/json");
        m_request.body = body.c_str();

        for (auto const& i : m_headers) {
            m_request.set(i.first, i.second);
        }
    }
}

void HttpSession::onWriteFinished(errorCode ec)
{
    if (ec) {
        mainLog()->debug("http session write error: {}", ec.message());
        requestFailed(this, ec.message());
        return;
    }

    http::async_read(
        m_tcpClient.stream(),
        m_readBuffer,
        m_response,
        std::bind(
            &HttpSession::onReadFinished,
            this,
            std::placeholders::_1));
}

void HttpSession::onReadFinished(errorCode ec)
{
    if (ec) {
        mainLog()->debug("http session read error: {}", ec.message());
        requestFailed(this, ec.message());
        return;
    }

    if (m_response.result() != http::status::ok) {
        requestFailed(this, m_response.body);
        return;
    }

    requestSuccess(this, m_response.body);
}
