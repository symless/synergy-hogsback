#include "ServiceWorker.h"

#include "synergy/common/RpcManager.h"
#include "synergy/common/WampRouter.h"
#include "synergy/common/WampServer.h"
#include "ProcessManager.h"
#include <boost/asio.hpp>
#include <iostream>

ServiceWorker::ServiceWorker(boost::asio::io_service &threadIoService) :
    m_threadIoService(threadIoService),
    m_work(std::make_shared<boost::asio::io_service::work>(threadIoService))
{
}

ServiceWorker::~ServiceWorker()
{
    m_work.reset();
    m_threadIoService.stop();
}

void ServiceWorker::start()
{
    if (m_processManager) {
        m_processManager = std::make_unique<ProcessManager>(m_threadIoService);
        m_processManager->onOutput.connect([](std::string line) {
            std::cout << line << std::endl;
        });
    }

    if (m_rpcManager) {
        m_rpcManager = std::make_unique<RpcManager>(m_threadIoService);
        m_rpcManager->initRouterAndServer();
        m_rpcManager->startRouter();
        m_rpcManager->getServer()->startCore.connect([this] (std::vector<std::string> cmd) { m_processManager->start(cmd); });
    }

    m_threadIoService.run();
}

void ServiceWorker::shutdown()
{
    m_processManager->shutdown();
    m_rpcManager->shutdown();

    m_work.reset();
    //finish processing all of the remaining completion handlers
    m_threadIoService.poll();
    m_threadIoService.stop();
}
