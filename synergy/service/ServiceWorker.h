#ifndef SERVICEWORKER_H
#define SERVICEWORKER_H

#include <synergy/service/router/Router.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>

class RpcManager;
class CoreManager;
class CloudClient;
class UserConfig;
class ProfileConfig;
class SessionMonitor;
class ErrorNotifier;
class RouterErrorMonitor;
class TrayService;

class ServiceWorker final
{
public:
    ServiceWorker(boost::asio::io_service& ioService,
                           std::shared_ptr<UserConfig> userConfig);
    ~ServiceWorker();

    void start();
    void shutdown();

private:
    void provideRpcEndpoints();
    void provideControls();
    void provideCore();
    void provideAuth();
    void provideSnapshot();
    void provideHello();
    void provideCloud();
    void provideTray();
    void provideLogging();
    void provideServerClaim();

private:
    boost::asio::io_service&        m_ioService;
    std::shared_ptr<UserConfig>     m_userConfig;
    std::shared_ptr<ProfileConfig>  m_remoteProfileConfig;
    std::shared_ptr<ProfileConfig>  m_localProfileConfig;
    std::unique_ptr<RpcManager>     m_rpc;
    std::shared_ptr<CloudClient>    m_cloudClient;
    Router                          m_router;
    std::unique_ptr<RouterErrorMonitor>
                                    m_routerMonitor;
    std::unique_ptr<CoreManager>    m_coreManager;
    std::unique_ptr<SessionMonitor> m_sessionMonitor;
    std::shared_ptr<boost::asio::io_service::work> m_work;
    boost::signals2::connection     m_logSender;
    std::string                     m_lastProfileSnapshot;
    std::unique_ptr<ErrorNotifier>  m_errorNotifier;
    std::unique_ptr<TrayService>    m_trayService;
};

#endif // SERVICEWORKER_H
