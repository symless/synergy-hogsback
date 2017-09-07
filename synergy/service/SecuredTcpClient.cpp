#include "SecuredTcpClient.h"

#include <boost/asio/connect.hpp>
#include <synergy/service/Logs.h>

SecuredTcpClient::SecuredTcpClient(boost::asio::io_service &ioService, std::string hostname, std::string port) :
    m_ioService(ioService),
    m_sslContext(ssl::context::tls_client),
    m_session(ioService, m_sslContext),
    m_resolver(ioService),
    m_address(hostname),
    m_port(port),
    m_connecting(false)
{
    m_session.connected.connect([this](SecuredTcpSession*){
        connected(this);
    });

    m_session.connectFailed.connect([this](SecuredTcpSession*){
        connectFailed(this);
    });
}

void SecuredTcpClient::connect()
{
    if (m_connecting) {
        mainLog()->warn("tcp client already in connecting");
        return;
    }

    m_resolver.async_resolve(
        {m_address, m_port},
        std::bind(
            &SecuredTcpClient::onResolveFinished,
            this,
            std::placeholders::_1,
                    std::placeholders::_2));
}

ssl::stream<tcp::socket> &SecuredTcpClient::stream()
{
    return m_session.stream();
}

void SecuredTcpClient::onResolveFinished(errorCode ec, tcp::resolver::iterator result)
{
    if (ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        mainLog()->debug("tcp client resolve error: {}", ec.message());

        connectFailed(this);
        return;
    }

    boost::asio::async_connect(
        m_session.stream().next_layer(),
        result,
        std::bind(
            &SecuredTcpClient::onConnectFinished,
            this,
            std::placeholders::_1));
}

void SecuredTcpClient::onConnectFinished(errorCode ec)
{
    if (ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        mainLog()->debug("tcp client connect error: {}", ec.message());

        connectFailed(this);
        return;
    }

    m_session.stream().async_handshake(
        ssl::stream_base::client,
        std::bind(
            &SecuredTcpClient::onSslHandshakeFinished,
            this,
                    std::placeholders::_1));
}

void SecuredTcpClient::onSslHandshakeFinished(errorCode ec)
{
    if (ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        mainLog()->debug("tcp session ssl handshake error: {}", ec.message());

        connectFailed(this);
        return;
    }

    m_connecting = true;
    connected(this);
}

std::string SecuredTcpClient::address() const
{
    return m_address;
}
