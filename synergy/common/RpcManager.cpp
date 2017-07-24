#include "RpcManager.h"
#include "RpcServer.h"
#include "WampRouter.h"

#include <iostream>

const char* const kLocalIpAddress = "127.0.0.1";
const int kWampDefaultPort = 24888;

RpcManager::RpcManager(boost::asio::io_service& ioService) :
    m_io(ioService)
{
    m_router = std::make_shared<WampRouter>(m_io, kLocalIpAddress, kWampDefaultPort);
    m_server = std::make_shared<RpcServer>(m_io, kLocalIpAddress, kWampDefaultPort);

    m_router->ready.connect ([this]() {
        m_io.post([this] () {
            m_server->start();
        });
    });

    m_server->ready.connect ([this]() {
        ready();
    });
}

RpcManager::~RpcManager()
{
}

void RpcManager::start()
{
    m_router->start();
}

void RpcManager::stop()
{
    m_router->stop();
}
