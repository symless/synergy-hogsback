#include <synergy/common/RpcManager.h>
#include <synergy/common/WampRouter.h>
#include <synergy/common/WampServer.h>

RpcManager::RpcManager (boost::asio::io_service& ioService, std::string ipAddress, int port) :
    m_ioService (ioService),
    m_router (std::make_shared<WampRouter>(m_ioService)),
    m_server (std::make_shared<WampServer>(m_ioService)),
    m_ipAddress(ipAddress),
    m_port(port)
{
    m_router->ready.connect ([this]() {
        m_ioService.post([this] () { m_server->start(m_ipAddress, m_port); });
    });
    m_server->ready.connect ([this]() { ready(); });
}

RpcManager::~RpcManager() noexcept
{}

void
RpcManager::start()
{
    m_router->start ("127.0.0.1", this->port());
}

void
RpcManager::stop()
{
    m_router->stop();
}

std::shared_ptr<WampServer>
RpcManager::server() {
    return m_server;
}

std::string
RpcManager::ipAddress() const
{
    return m_ipAddress;
}

int
RpcManager::port() const
{
    return m_port;
}
