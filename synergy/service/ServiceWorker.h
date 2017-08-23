#ifndef SERVICEWORKER_H
#define SERVICEWORKER_H

#include <boost/asio/io_service.hpp>

class RpcManager;
class ProcessManager;
class CloudClient;
class UserConfig;

class ServiceWorker
{
public:
    ServiceWorker(boost::asio::io_service& ioService);
    ~ServiceWorker();

    void start();
    void shutdown();

private:
    void provideRpcEndpoints();
    void provideCore();
    void provideAuthUpdate();
    boost::asio::io_service& m_ioService;
    std::unique_ptr<RpcManager> m_rpcManager;
    std::unique_ptr<ProcessManager> m_processManager;
    std::shared_ptr<boost::asio::io_service::work> m_work;
    std::shared_ptr<CloudClient> m_cloudClient;
    std::shared_ptr<UserConfig> m_userConfig;
};

#endif // SERVICEWORKER_H
