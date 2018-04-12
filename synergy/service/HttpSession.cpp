#include "HttpSession.h"

#include <synergy/service/ServiceLogs.h>
#include <boost/asio/connect.hpp>

static const int kHttpVersion = 11;

HttpSession::HttpSession(boost::asio::io_service& ioService, std::string hostname, std::string port) :
    m_readBuffer(2048),
    m_ioService(ioService),
    m_hostname(hostname),
    m_port(port)
{
    addHeader("X-Synergy-Version", SYNERGY_VERSION_STRING);
}

void HttpSession::addHeader(std::string headerName, std::string headerContent)
{
    m_headers[headerName] = headerContent;
}

void HttpSession::get(const std::string &target)
{
    setupRequest(http::verb::get, target);

    send();
}

void HttpSession::post(const std::string &target, const std::string &body)
{
    setupRequest(http::verb::post, target, body);

    send();
}

void HttpSession::send()
{
    if (!m_tcpClient) {
        m_tcpClient = std::make_unique<SecuredTcpClient>(m_ioService, m_hostname, m_port);

        m_tcpClient->connected.connect(
            [this](SecuredTcpClient*) {
                onTcpClientConnected();
            },
            boost::signals2::at_front
        );

        m_tcpClient->connectFailed.connect(
            [this](errorCode ec){
            requestFailed(ec);
        });

        m_tcpClient->connect();
    }
    else {
        sendRequest();
    }
}

void HttpSession::sendRequest()
{
    http::async_write(m_tcpClient->stream(), m_request,
        std::bind(
            &HttpSession::onWriteFinished,
            this,
            std::placeholders::_1));
}

void HttpSession::onTcpClientConnected()
{
    sendRequest();
}

void HttpSession::setupRequest(http::verb method, const std::string &target, const std::string &body)
{
    m_request.version(kHttpVersion);
    m_request.method(method);
    m_request.target(std::move(target));
    m_request.set(http::field::host, m_hostname.c_str());
    m_request.set(http::field::connection, "keep-alive");
    m_response = http::response<http::string_body>();

    if (!body.empty()) {
        m_request.set(http::field::content_type, "application/json");
        m_request.body() = body;

        for (auto const& i : m_headers) {
            m_request.set(i.first, i.second);
        }

        m_request.prepare_payload();
    }
}

void HttpSession::onWriteFinished(errorCode ec)
{
    if (ec) {
        serviceLog()->debug("http session write error: {}", ec.message());
        m_tcpClient.reset();
        requestFailed(ec);
        return;
    }

    http::async_read(
        m_tcpClient->stream(),
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
        serviceLog()->debug("http session read error: {}", ec.message());
        m_tcpClient.reset();
        requestFailed(ec);
        return;
    }

    responseReceived(m_response.result(), m_response.body());
}
