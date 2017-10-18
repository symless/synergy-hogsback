#include "SecuredTcpServer.h"
#include <synergy/service/ServiceLogs.h>

SecuredTcpServer::SecuredTcpServer(boost::asio::io_service &ioService) :
    m_ioService(ioService),
    m_sslContext(ssl::context::tls_server),
    m_acceptor(ioService)
{
}

void SecuredTcpServer::start()
{
    boost::system::error_code ec;
    tcp::endpoint endpoint(
                boost::asio::ip::address::from_string(m_address.c_str()),
                std::stoi(m_port));
    m_acceptor.open(endpoint.protocol(), ec);
    if (ec) {
        serviceLog()->debug("tcp server open error: {}", ec.message());
        startFailed(this);
    }

    m_acceptor.bind(endpoint, ec);
    if (ec) {
        serviceLog()->debug("tcp server bind error: {}", ec.message());
        startFailed(this);
    }

    m_acceptor.listen(boost::asio::socket_base::max_connections, ec);
    if (ec) {
        serviceLog()->debug("tcp server listen error: {}", ec.message());
        startFailed(this);
    }

    accept();

    started(this);
}

std::string SecuredTcpServer::address() const
{
    return m_address;
}

void SecuredTcpServer::setAddress(const std::string &address)
{
    m_address = address;
}

std::string SecuredTcpServer::port() const
{
    return m_port;
}

void SecuredTcpServer::setPort(const std::string &port)
{
    m_port = port;
}

void SecuredTcpServer::loadRawCetificate(const std::string& cert, const std::string& key, const std::string& dh)
{
    m_sslContext.set_options(
                boost::asio::ssl::context::default_workarounds |
                boost::asio::ssl::context::no_sslv3 |
                boost::asio::ssl::context::single_dh_use);

    m_sslContext.use_certificate_chain(
            boost::asio::buffer(cert.data(), cert.size()));

    m_sslContext.use_private_key(
        boost::asio::buffer(key.data(), key.size()),
        boost::asio::ssl::context::file_format::pem);

    m_sslContext.use_tmp_dh(
        boost::asio::buffer(dh.data(), dh.size()));

    m_sslContext.set_password_callback(
        [](std::size_t,
            boost::asio::ssl::context_base::password_purpose)
        {
            return "test";
        });
}

void SecuredTcpServer::accept()
{
    SecuredTcpSession* session = new SecuredTcpSession(m_ioService, m_sslContext);

    m_acceptor.async_accept(
                    session->stream().next_layer(),
                    std::bind(
                        &SecuredTcpServer::onAccept,
                        this,
                        session,
                        std::placeholders::_1));
}

void SecuredTcpServer::onAccept(SecuredTcpSession* session, boost::system::error_code ec)
{
    if (ec) {
        serviceLog()->debug("tcp server accept error: {}", ec.message());
        acceptFailed(this);
        delete session;
    }
    else {
        auto address = session->stream().lowest_layer().remote_endpoint().address().to_string();
        serviceLog()->debug("tcp server accepted connection, address={}", address);

        session->stream().async_handshake(
            ssl::stream_base::server,
            std::bind(
                &SecuredTcpServer::onSslHandshakeFinished,
                this,
                session,
                std::placeholders::_1));
    }

    // accept next client
    serviceLog()->debug("tcp server waiting for next client");
    accept();
}

void SecuredTcpServer::onSslHandshakeFinished(SecuredTcpSession* session, errorCode ec)
{
    if (ec) {
        serviceLog()->debug("tcp server ssl handshake error: {}", ec.message());
        acceptFailed(this);
        delete session;
        return;
    }

    auto address = session->stream().lowest_layer().remote_endpoint().address().to_string();
    serviceLog()->debug("tcp server ssl handshake finished, address={}", address);
    accepted(session, address);

    m_sessions.emplace_back(session);
}
