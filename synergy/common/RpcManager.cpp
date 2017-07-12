#include "RpcManager.h"

#include "WampServer.h"
#include "WampRouter.h"

#include <iostream>

const char* const kLocalIpAddress = "127.0.0.1";
const int kWampDefaultPort = 24888;

RpcManager::RpcManager(boost::asio::io_service& ioService) :
    m_mainIoService(ioService)
{
}

RpcManager::~RpcManager()
{
    m_routerThread.join();

}

void RpcManager::initRouterAndServer()
{
    m_server = std::make_shared<WampServer>();
    m_router = std::make_shared<WampRouter>(kLocalIpAddress, kWampDefaultPort);
    m_router->ready.connect([this]() {
        m_mainIoService.post([this] () {
            m_server->start(m_mainIoService, kLocalIpAddress, kWampDefaultPort);
        });
    });
}

void RpcManager::startRouter()
{
    m_routerThread = std::thread([this] () { m_router->run(); });
}
