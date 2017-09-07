#ifndef SERVICEWORKER_H
#define SERVICEWORKER_H

#include <boost/asio/io_service.hpp>

class RpcManager;
class ProcessManager;
class ConnectivityTester;
class CloudClient;
class UserConfig;
class Profile;

class ServiceWorker final
{
public:
    explicit ServiceWorker(boost::asio::io_service& ioService,
                           std::shared_ptr<UserConfig> userConfig);
    ~ServiceWorker();

    void start();
    void shutdown();

private:
    void provideRpcEndpoints();
    void provideCore();
    void provideAuthUpdate();

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<UserConfig> m_userConfig;
    std::shared_ptr<Profile> m_activeProfile;
    std::shared_ptr<boost::asio::io_service::work> m_work;
    std::unique_ptr<RpcManager> m_rpcManager;
    std::unique_ptr<ProcessManager> m_processManager;
    std::unique_ptr<ConnectivityTester> m_connectivityTester;
    std::unique_ptr<CloudClient> m_cloudClient;
};

#endif // SERVICEWORKER_H
