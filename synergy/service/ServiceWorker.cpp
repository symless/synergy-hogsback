#include <synergy/service/ServiceWorker.h>

#include <synergy/service/CloudClient.h>
#include <synergy/service/ServiceLogs.h>
#include <synergy/service/CoreManager.h>
#include <synergy/service/ErrorNotifier.h>
#include <synergy/service/CoreErrorMonitor.h>
#include <synergy/service/RouterErrorMonitor.h>
#include <synergy/service/SessionMonitor.h>
#include <synergy/service/WebsocketError.h>
#include <synergy/service/TrayService.h>
#include <synergy/service/router/protocol/v2/MessageTypes.hpp>
#include <synergy/common/UserConfig.h>
#include <synergy/common/RpcManager.h>
#include <synergy/common/WampServer.h>
#include <synergy/common/WampRouter.h>
#include <synergy/common/ScreenStatus.h>
#include <synergy/common/Profile.h>
#include <synergy/common/ProfileConfig.h>
#include <synergy/common/NetworkParameters.h>

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

ServiceWorker::ServiceWorker(boost::asio::io_service& ioService,
                             std::shared_ptr<UserConfig> userConfig) :
    m_ioService (ioService),
    m_userConfig (std::move(userConfig)),
    m_remoteProfileConfig (std::make_shared<ProfileConfig>()),
    m_localProfileConfig (std::make_shared<ProfileConfig>()),
    m_rpc (std::make_unique<RpcManager>(m_ioService)),
    m_cloudClient (std::make_shared<CloudClient>(ioService, m_userConfig, m_remoteProfileConfig)),
    m_router (ioService, kNodePort),
    m_routerMonitor(std::make_unique<RouterErrorMonitor>(m_localProfileConfig, m_router)),
    m_coreManager (std::make_unique<CoreManager>(m_ioService, m_userConfig, m_localProfileConfig, m_cloudClient, *m_rpc, m_router)),
    m_sessionMonitor (std::make_unique<SessionMonitor>(ioService)),
    m_work (std::make_shared<boost::asio::io_service::work>(ioService)),
    m_errorNotifier(std::make_unique<ErrorNotifier>(*m_cloudClient, *m_localProfileConfig, *m_userConfig)),
    m_trayService(std::make_unique<TrayService>(m_ioService))

{
    m_localProfileConfig->modified.connect([this](){
        serviceLog()->debug("local profile modified, id={}", m_localProfileConfig->profileId());
        m_remoteProfileConfig->compare(*m_localProfileConfig);
    });

    m_cloudClient->websocketMessageReceived.connect([this](std::string json){
        try {
            // parse json message and copy into remote profile config
            ProfileConfig profileConfig = ProfileConfig::fromJsonSnapshot(json);
            m_remoteProfileConfig->clone(profileConfig);
        }
        catch (const std::exception& ex) {
            serviceLog()->error("failed to create profile from json: {}", ex.what());
            return;
        }

        // store profile config (causes signals to be invoked)
        m_localProfileConfig->apply(*m_remoteProfileConfig);

        // save for future requests from config UI
        m_lastProfileSnapshot = json;

        // forward the message via rpc server to config UI
        auto rpcServer = m_rpc->server();
        rpcServer->publish("synergy.profile.snapshot", std::move(json));
    });

    m_cloudClient->websocketConnected.connect([this](){
        serviceLog()->debug("cloud client connected");
        m_rpc->server()->publish("synergy.cloud.online");
    });

    m_cloudClient->websocketError.connect([this](WebsocketError ec){

        serviceLog()->debug("clearing last profile snapshot");
        m_lastProfileSnapshot.clear();

        m_rpc->server()->publish("synergy.cloud.offline");

        if (ec == WebsocketError::kAuth) {
            // Authentication failed either by using an invalid session or an out of date version
            serviceLog()->debug("sending logout message to config ui");
            m_rpc->server()->publish("synergy.auth.logout");
        }
    });

    m_rpc->ready.connect([this]() {
        provideRpcEndpoints();

        m_logSender = g_serviceLog.onLogLine.connect([this](std::string logLine) {
            try {
                auto server = m_rpc->server();
                server->publish ("synergy.service.log", std::move(logLine));
            }
            catch (const std::exception& ex) {
                m_logSender.disconnect();
                serviceLog()->error("failed to send log to config ui: {}", ex.what());
            }
        });
    });

    m_sessionMonitor->activeUserChanged.connect([this](std::string user) {
        this->m_coreManager->setRunAsUid (std::move (user));
    });

    m_sessionMonitor->activeDisplayChanged.connect([this](std::string display) {
        this->m_coreManager->setDisplay (std::move (display));
    });

#if __linux__
    std::string coreUid = m_userConfig->systemUid();
    if (!coreUid.empty()) {
        serviceLog()->debug("setting core uid from config: {}", coreUid);
        m_coreManager->setRunAsUid(coreUid);
    }
    else {
        serviceLog()->warn("core uid is not set in config file");
    }
#endif

    m_localProfileConfig->screenOnline.connect([this](Screen screen){
        if (m_userConfig->screenId() == screen.id()) {
            return;
        }

        m_ioService.post([this, screen] () {
            std::vector<std::string> ipList;
            std::string ipListStr = screen.ipList();

            boost::split(ipList, ipListStr, boost::is_any_of(","));

            for(const auto& ipStr : ipList) {
                m_router.add_peer (tcp::endpoint
                                   (ip::address::from_string (ipStr), kNodePort));
            }
        });
    });

    m_errorNotifier->install(m_coreManager->errorMonitor());
    m_errorNotifier->install(m_coreManager->statusMonitor());
    m_errorNotifier->install(*m_routerMonitor);

    m_sessionMonitor->start();
    m_rpc->start();
}

ServiceWorker::~ServiceWorker()
{
    m_work.reset();
    m_ioService.stop();
    m_userConfig->save();
    m_logSender.disconnect();
}

void
ServiceWorker::start()
{
    m_ioService.run();
}

void
ServiceWorker::shutdown()
{
    m_sessionMonitor->stop();
    m_coreManager->shutdown();
    m_rpc->stop();
    m_work.reset();

    // Finish processing all of the remaining completion handlers
    m_ioService.poll();
    m_ioService.stop();
}

void
ServiceWorker::provideRpcEndpoints()
{
    serviceLog()->debug("creating rpc endpoints");

    provideCore();
    provideAuth();
    provideSnapshot();
    provideHello();
    provideCloud();
    provideLogging();
    provideServerClaim();
    provideTray();

    serviceLog()->debug("rpc endpoints created");
}

void
ServiceWorker::provideCore()
{
    auto server = m_rpc->server();

    server->provide ("synergy.core.set_uid",
        [this](std::string uid) {
        serviceLog()->debug("setting core uid from rpc: {}", uid);
        m_userConfig->setSystemUid(uid);
        m_coreManager->setRunAsUid(uid);
    });
}

void
ServiceWorker::provideAuth()
{
    m_rpc->server()->provide(
        "synergy.auth.update",
        [this](int userId, int screenId, int profileId, std::string userToken) {

        m_userConfig->setUserId(userId);
        m_userConfig->setScreenId(screenId);
        m_userConfig->setProfileId(profileId);
        m_userConfig->setUserToken(std::move(userToken));
        m_userConfig->save();

        serviceLog()->debug("got user auth token: {}", m_userConfig->userToken());
    });
}

void ServiceWorker::provideSnapshot()
{
    m_rpc->server()->provide(
        "synergy.snapshot.request",
        [this]() {

        if (!m_lastProfileSnapshot.empty()) {
            serviceLog()->debug("sending last profile snapshot");
            m_rpc->server()->publish("synergy.profile.snapshot", m_lastProfileSnapshot);
        }
        else {
            serviceLog()->error("can't send profile snapshot, not yet received from cloud");
        }
    });
}

void
ServiceWorker::provideHello()
{
    m_rpc->server()->provide(
        "synergy.hello", [this]() {

        serviceLog()->debug("saying hello to config ui");

        if (!m_cloudClient->isWebsocketConnected() &&
            m_userConfig->profileId() != -1) {
            m_rpc->server()->publish("synergy.cloud.offline");

            m_cloudClient->reconnectWebsocket();
        }

        if (m_userConfig->versionCheck()) {
            m_rpc->server()->publish("synergy.version.check");
        }
    });
}

void
ServiceWorker::provideCloud()
{
    m_rpc->server()->provide(
        "synergy.cloud.retry", [this]() {

        serviceLog()->debug("retrying cloud connection");
        m_cloudClient->reconnectWebsocket();
    });
}

void
ServiceWorker::provideTray()
{
    m_rpc->server()->provide ("synergy.log.tray", [this](std::string logLine) {
        boost::algorithm::trim_right (logLine);
        trayLog()->debug(logLine);
    });

    m_rpc->server()->provide ("synergy.tray.hello", [this]() {
        //serviceLog()->info("Tray process connected");
        bool const kill = !m_trayService->start();
        if (kill) {
            serviceLog()->info ("A tray process is already running. Responding "
                                "with kill command");
        }
        return kill;
    });

    m_rpc->server()->provide ("synergy.tray.goodbye", [this]() {
        m_trayService->stop();
    });

    m_rpc->server()->provide ("synergy.tray.ping", [this]() {
        serviceLog()->debug ("Received ping from tray");
        m_trayService->ping();
    });
}

void
ServiceWorker::provideLogging()
{
    m_rpc->server()->provide(
        "synergy.log.config", [this](std::string logLine) {
        configLog()->debug(logLine);
    });
}

void ServiceWorker::provideServerClaim()
{
    m_rpc->server()->provide(
        "synergy.server.claim", [this](int serverId) {
        m_coreManager->switchServer(serverId);
        m_coreManager->notifyServerClaim(serverId);
    });
}
