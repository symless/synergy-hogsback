#ifndef SERVICEWORKER_H
#define SERVICEWORKER_H

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>

class RpcManager;
class ProcessManager;
class CloudClient;
class UserConfig;
class ProfileConfig;

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
    void provideProfileRequest();
    void provideCloud();

private:
    boost::asio::io_service&        m_ioService;
    std::shared_ptr<UserConfig>     m_userConfig;
    std::shared_ptr<ProfileConfig>  m_remoteProfileConfig;
    std::shared_ptr<ProfileConfig>  m_localProfileConfig;
    std::unique_ptr<RpcManager>     m_rpc;
    std::unique_ptr<CloudClient>    m_cloudClient;
    std::unique_ptr<ProcessManager> m_processManager;
    std::shared_ptr<boost::asio::io_service::work> m_work;
    boost::asio::deadline_timer m_testTimer;
};

#endif // SERVICEWORKER_H
