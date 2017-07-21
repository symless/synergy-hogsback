#include "ServiceWorker.h"

#include "synergy/common/RpcManager.h"
#include "synergy/common/WampRouter.h"
#include "synergy/common/WampServer.h"
#include <synergy/service/ProcessManager.h>
#include <iostream>

ServiceWorker::ServiceWorker(boost::asio::io_service &threadIoService) :
    m_threadIoService(threadIoService)
{
}

void ServiceWorker::start()
{
    ProcessManager processManager(m_threadIoService);
    processManager.onOutput.connect([](std::string line) {
        std::cout << line << std::endl;
    });

    RpcManager rpcManager(m_threadIoService);
    rpcManager.initRouterAndServer();
    rpcManager.startRouter();
    rpcManager.getServer()->startCore.connect([&processManager] (std::vector<std::string> cmd) { processManager.start(cmd); });

    boost::asio::io_service::work work(m_threadIoService);
    m_threadIoService.run();
}
