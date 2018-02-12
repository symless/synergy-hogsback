#include <synergy/common/RpcManager.h>
#include <synergy/common/WampRouter.h>
#include <synergy/common/WampServer.h>
#include <synergy/service/ServiceLogs.h>

RpcManager::RpcManager (boost::asio::io_service& ioService,
                        std::string ip, int const port) :
    m_ioService (ioService),
    m_router (std::make_shared<WampRouter>(ioService, serviceLog())),
    m_server (std::make_shared<WampServer>(ioService)),
    m_ip (std::move (ip)),
    m_port (port)
{
    m_router->ready.connect ([this]() {
        m_ioService.post([this] () {
            this->server()->start(m_ip, m_port);
        });
    });

    m_server->ready.connect ([this]() {
        this->ready();
    });
}

RpcManager::~RpcManager() noexcept {
}

void
RpcManager::start() {
    m_router->start ("127.0.0.1", this->port());
}

void
RpcManager::stop() {
    m_router->stop();
}

std::shared_ptr<WampServer>
RpcManager::server() {
    return m_server;
}

std::string
RpcManager::ip() const {
    return m_ip;
}

int
RpcManager::port() const {
    return m_port;
}
