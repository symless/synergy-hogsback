#ifndef SERVICEWORKER_H
#define SERVICEWORKER_H

#include "synergy/service/IOService.h"

class ProcessManager;
class RpcManager;

class ServiceWorker
{
public:
    ServiceWorker(boost::asio::io_service& threadIoService);
    ~ServiceWorker();

    void start();
    void shutdown();

private:
    boost::asio::io_service& m_threadIoService;
    std::shared_ptr<boost::asio::io_service::work> m_work;
    std::unique_ptr<ProcessManager> m_processManager;
    std::unique_ptr<RpcManager> m_rpcManager;
};

#endif // SERVICEWORKER_H
