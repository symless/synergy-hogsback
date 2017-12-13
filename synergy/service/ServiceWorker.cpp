#include "ServiceWorker.h"

#include <synergy/common/ProfileConfig.h>
#include <synergy/service/CloudClient.h>
#include <synergy/service/ServiceLogs.h>
#include <synergy/common/UserConfig.h>
#include <synergy/common/RpcManager.h>
#include <synergy/common/WampServer.h>
#include <synergy/common/WampRouter.h>
#include <synergy/common/ScreenStatus.h>
#include <synergy/common/Profile.h>
#include <synergy/common/NetworkParameters.h>
#include <synergy/common/ProcessCommand.h>

#include <synergy/service/CoreProcess.h>
#include <synergy/service/WebsocketError.h>
#include <synergy/service/router/protocol/v2/MessageTypes.hpp>

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
    m_cloudClient (std::make_unique<CloudClient>(ioService, m_userConfig, m_remoteProfileConfig)),
    m_processCommand(std::make_shared<ProcessCommand>()),
    m_router (ioService, kNodePort),
    m_coreProcess (std::make_unique<CoreProcess>(m_ioService, m_userConfig, m_localProfileConfig, m_router, m_processCommand)),
    m_serverProxy (ioService, m_router, kServerProxyPort),
    m_clientProxy (ioService, m_router, kServerPort),
    m_work (std::make_shared<boost::asio::io_service::work>(ioService))
{
    m_processCommand->setLocalHostname(boost::asio::ip::host_name());

    g_commonLog.onLogLine.connect([this](std::string logLine) {
        auto server = m_rpc->server();
        server->publish ("synergy.service.log", std::move(logLine));
    });

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

        // HACK: say that the cloud server is online when there is a snapshot
        m_rpc->server()->publish("synergy.cloud.online");
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

    m_coreProcess->localInputDetected.connect([this](){
        m_ioService.post([this] () {
            m_coreProcess->switchServer(m_userConfig->screenId());
            m_coreProcess->broadcastServerClaim(m_userConfig->screenId());
        });

        serviceLog()->debug("local input detected, claiming this computer as server in local network");
    });

    m_coreProcess->serverReady.connect([this](){
        m_cloudClient->claimServer();
    });

#if __linux__
    std::string coreUid = m_userConfig->systemUid();
    if (!coreUid.empty()) {
        serviceLog()->debug("setting core uid from config: {}", coreUid);
        m_processCommand->setRunAsUid(coreUid);
    }
    else {
        serviceLog()->debug("core uid is unknown");
    }
#endif

    m_rpc->start();

    if (m_userConfig->screenId() != -1) {
        m_router.start (m_userConfig->screenId(), boost::asio::ip::host_name());
        m_clientProxy.start ();
    }
    else {
        m_userConfig->updated.connect_extended ([this](const auto& connection) {
            if (m_userConfig->screenId() != -1) {
                m_router.start (m_userConfig->screenId(), boost::asio::ip::host_name());
                m_clientProxy.start ();
                connection.disconnect();
            }
        });
    }

    m_localProfileConfig->profileServerChanged.connect
        ([this](int64_t const server) { m_serverProxy.start (server); });

    m_localProfileConfig->screenOnline.connect([this](Screen screen){
        if (m_userConfig->screenId() == screen.id()) {
            return;
        }

        m_ioService.post([this, screen] () {
            std::vector<std::string> ipList;
            std::string ipListStr = screen.ipList();

            boost::split(ipList, ipListStr, boost::is_any_of(","));

            for(const auto& ipStr : ipList) {
                ip::address ip = ip::address::from_string (ipStr);
                m_router.add(tcp::endpoint (ip, kNodePort));
            }
        });
    });
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
    m_coreProcess->shutdown();
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
    provideLog();
    provideServerClaim();

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
        m_processCommand->setRunAsUid(uid);
    });

    m_coreProcess->output.connect(
        [server](std::string line) {
            coreLog()->debug(line);
            server->publish ("synergy.core.log", std::move(line));
        }
    );

    m_coreProcess->screenStatusChanged.connect(
        [server](std::string const& screenName, ScreenStatus state) {
            server->publish ("synergy.screen.status", screenName, int(state));
        }
    );

    m_coreProcess->screenConnectionError.connect(
        [server](std::string const& screenName) {

            // TODO: get error code
            int ec = 0;
            server->publish ("synergy.screen.error", screenName,
                                (int)ec);
            server->publish ("synergy.screen.status", screenName,
                                (int)ScreenStatus::kConnectingWithError);
        }
    );
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

            // HACK: say that the cloud server is online when there is a snapshot
            m_rpc->server()->publish("synergy.cloud.online");
        }
        else {
            serviceLog()->error("can't send profile snapshot, not yet received from cloud");

            // HACK: say that the cloud server is offline when there's no snapshot
            m_rpc->server()->publish("synergy.cloud.offline");
        }
    });
}

void
ServiceWorker::provideHello()
{
    m_rpc->server()->provide(
        "synergy.hello", [this]() {

        serviceLog()->debug("saying hello to config ui");

        if (!m_cloudClient->isWebsocketConnected()) {
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
ServiceWorker::provideLog()
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
        m_serverProxy.start(serverId);
        m_coreProcess->switchServer(serverId);
        m_coreProcess->broadcastServerClaim(serverId);
    });
}
