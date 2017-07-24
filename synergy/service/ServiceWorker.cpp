#include "ServiceWorker.h"

#include <synergy/common/RpcManager.h>
#include <synergy/common/RpcServer.h>
#include <synergy/common/WampRouter.h>
#include "ProcessManager.h"
#include <boost/asio.hpp>
#include <iostream>

ServiceWorker::ServiceWorker(boost::asio::io_service &threadIoService) :
    m_ioService(threadIoService),
    m_work(std::make_shared<boost::asio::io_service::work>(threadIoService))
{
    m_processManager = std::make_unique<ProcessManager>(m_ioService);
    m_processManager->onOutput.connect([](std::string line) {
        std::cout << line << std::endl;
    });

    m_rpcManager = std::make_unique<RpcManager>(m_ioService);
    m_rpcManager->ready.connect([this]() { provideCore(); });
    m_rpcManager->start();
}

ServiceWorker::~ServiceWorker()
{
    m_work.reset();
    m_ioService.stop();
}

void ServiceWorker::start()
{
    m_ioService.run();
}

void
ServiceWorker::provideCore()
{
    m_rpcManager->provide ("synergy.core.start",
                           [this](std::vector<std::string> cmd) {
        m_processManager->start(std::move (cmd));
    });
}

void ServiceWorker::shutdown()
{
    m_processManager->stop();
    m_rpcManager->stop();

    m_work.reset();
    //finish processing all of the remaining completion handlers
    m_ioService.poll();
    m_ioService.stop();
}
