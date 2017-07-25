#include <synergy/common/RpcManager.h>
#include <synergy/common/WampRouter.h>
#include <synergy/common/WampServer.h>

const char* const kLocalIpAddress = "127.0.0.1";
const int kWampDefaultPort = 24888;

RpcManager::RpcManager(boost::asio::io_service& ioService) :
    m_ioService(ioService)
{
    m_router = std::make_shared<WampRouter>(m_ioService);
    m_server = std::make_shared<WampServer>(m_ioService);

    m_router->ready.connect ([this]() {
        m_ioService.post([this] () { m_server->start(kLocalIpAddress, kWampDefaultPort); });
    });

    m_server->ready.connect ([this]() { ready(); });
}

RpcManager::~RpcManager()
{
}

void
RpcManager::start()
{
    m_router->start (kLocalIpAddress, kWampDefaultPort);
}

void
RpcManager::stop()
{
    m_router->stop();
}
