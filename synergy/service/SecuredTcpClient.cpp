#include "SecuredTcpClient.h"

#include <boost/asio/connect.hpp>
#include <synergy/service/ServiceLogs.h>

namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

SecuredTcpClient::SecuredTcpClient(boost::asio::io_service &ioService, std::string hostname, std::string port) :
    m_ioService(ioService),
    m_sslContext(ssl::context::tls_client),
    m_stream(ioService, m_sslContext),
    m_resolver(ioService),
    m_address(hostname),
    m_port(port),
    m_connected(false)
{
}

void SecuredTcpClient::connect()
{
    if (m_connected) {
        serviceLog()->warn("tcp client already connected");
        return;
    }

    // resolve syncronously due to a bug in boost where
    // m_resolver.cancel doesn't trigger abort.
    ErrorCode ec;
    auto result = m_resolver.resolve({m_address, m_port}, ec);
    onResolveFinished(ec, result);
}

SecuredTcpClient::Stream&
SecuredTcpClient::stream()
{
    return m_stream;
}

void SecuredTcpClient::onResolveFinished(ErrorCode ec, tcp::resolver::iterator result)
{
    if (ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        serviceLog()->debug("tcp client resolve error: {}", ec.message());

        connectFailed(this);
        return;
    }

    boost::asio::async_connect(
        this->stream().next_layer(),
        result,
        std::bind(
            &SecuredTcpClient::onConnectFinished,
            this,
            std::placeholders::_1));
}

void SecuredTcpClient::onConnectFinished(ErrorCode ec)
{
    if (ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        serviceLog()->debug("tcp client connect error: {}", ec.message());

        connectFailed(this);
        return;
    }

    this->stream().async_handshake(
        ssl::stream_base::client,
        std::bind(
            &SecuredTcpClient::onSslHandshakeFinished,
            this,
                    std::placeholders::_1));
}

void SecuredTcpClient::onSslHandshakeFinished(ErrorCode ec)
{
    if (ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        serviceLog()->debug("tcp session ssl handshake error: {}", ec.message());

        connectFailed(this);
        return;
    }

    m_connected = true;
    connected(this);
}

std::string SecuredTcpClient::address() const
{
    return m_address;
}

std::string SecuredTcpClient::port() const
{
    return m_port;
}
