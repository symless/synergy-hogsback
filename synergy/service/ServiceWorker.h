#ifndef SERVICEWORKER_H
#define SERVICEWORKER_H

#include <boost/asio/io_service.hpp>

class RpcManager;
class ProcessManager;

class ServiceWorker
{
public:
    ServiceWorker(boost::asio::io_service& ioService);
    ~ServiceWorker();

    void start();
    void shutdown();

private:
    void provideCore();
    boost::asio::io_service& m_ioService;
    std::unique_ptr<RpcManager> m_rpcManager;
    std::unique_ptr<ProcessManager> m_processManager;
    std::shared_ptr<boost::asio::io_service::work> m_work;
};

#endif // SERVICEWORKER_H
