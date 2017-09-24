#include "ServiceWorker.h"

#include <synergy/common/ProfileConfig.h>
#include "synergy/service/CloudClient.h"
#include <synergy/service/ServiceLogs.h>
#include <synergy/common/UserConfig.h>
#include <synergy/common/RpcManager.h>
#include <synergy/common/WampServer.h>
#include <synergy/common/WampRouter.h>
#include <synergy/common/ScreenStatus.h>
#include <synergy/common/Profile.h>
#include "ProcessManager.h"
#include <boost/asio.hpp>
#include <iostream>

std::string g_lastProfileSnapshot;

ServiceWorker::ServiceWorker(boost::asio::io_service& ioService,
                             std::shared_ptr<UserConfig> userConfig) :
    m_ioService (ioService),
    m_userConfig (std::move(userConfig)),
    m_remoteProfileConfig (std::make_shared<ProfileConfig>()),
    m_localProfileConfig (std::make_shared<ProfileConfig>()),
    m_rpcManager (std::make_unique<RpcManager>(m_ioService)),
    m_cloudClient (std::make_unique<CloudClient>(ioService, m_userConfig, m_remoteProfileConfig)),
    m_processManager (std::make_unique<ProcessManager>(m_ioService, m_userConfig, m_localProfileConfig)),
    m_work (std::make_shared<boost::asio::io_service::work>(ioService))
{
    g_serviceLog.onLogLine.connect([this](std::string logLine) {
        auto server = m_rpcManager->server();
        server->publish ("synergy.service.log", std::move(logLine));
    });

    g_commonLog.onLogLine.connect([this](std::string logLine) {
        auto server = m_rpcManager->server();
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
        // TODO: store json in profile config instead of global?
        g_lastProfileSnapshot = json;

        // forward the message via rpc server to config UI
        auto rpcServer = m_rpcManager->server();
        rpcServer->publish("synergy.profile.snapshot", std::move(json));

        // HACK: if no server, then use the first screen in the profile. this should
        // really be done on the cloud server.
        if (!m_localProfileConfig->hasServer()) {
            serviceLog()->debug("no server has been established yet, using first screen");
            auto screens = m_localProfileConfig->screens();
            if (!screens.empty()) {
                m_cloudClient->claimServer(screens[0].id());
            }
            else {
                serviceLog()->error("can't choose a server, no screens in config");
            }
        }
    });

    m_rpcManager->ready.connect([this]() {
        provideRpcEndpoints();
        serviceLog()->info("service started successfully");
    });

    m_processManager->localInputDetected.connect([this](){
        serviceLog()->debug("local input detected, claiming this computer as server");
        m_cloudClient->claimServer(m_userConfig->screenId());
    });

    m_rpcManager->start();
}

ServiceWorker::~ServiceWorker()
{
    m_work.reset();
    m_ioService.stop();
    m_userConfig->save();
}

void
ServiceWorker::start()
{
    m_ioService.run();
}

void
ServiceWorker::provideCore()
{
    auto server = m_rpcManager->server();

    server->provide ("synergy.core.start",
                     [this](std::vector<std::string>& cmd) {
        m_processManager->start (std::move (cmd));
    });

    server->provide ("synergy.profile.request",
                     [this](std::vector<std::string>& cmd) {
        if (g_lastProfileSnapshot.empty()) {
            serviceLog()->error("can't send profile snapshot, not yet received from cloud");
            return;
        }

        serviceLog()->debug("sending last profile snapshot");
        auto server = m_rpcManager->server();
        server->publish ("synergy.profile.snapshot", g_lastProfileSnapshot);

        // TODO: remove hack
        serviceLog()->debug("config ui opened, forcing connectivity test");
        m_localProfileConfig->forceConnectivityTest();
    });

    m_processManager->output.connect(
        [server](std::string line) {
            server->publish ("synergy.core.log", std::move(line));
        }
    );

    m_processManager->screenStatusChanged.connect(
        [server](std::string const& screenName, ScreenStatus state) {
            server->publish ("synergy.screen.status", screenName, int(state));
        }
    );

    m_processManager->screenConnectionError.connect(
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

void ServiceWorker::provideAuthUpdate()
{
    auto server = m_rpcManager->server();

    server->provide ("synergy.auth.update",
                     [this](int userId, int screenId, int profileId,
                     std::string userToken) {
        m_userConfig->setUserId(userId);
        m_userConfig->setScreenId(screenId);
        m_userConfig->setProfileId(profileId);
        m_userConfig->setUserToken(std::move(userToken));
        m_userConfig->save();
    });
}

void ServiceWorker::shutdown()
{
    m_processManager->shutdown();
    m_rpcManager->stop();
    m_work.reset();

    // Finish processing all of the remaining completion handlers
    m_ioService.poll();
    m_ioService.stop();
}

void ServiceWorker::provideRpcEndpoints()
{
    serviceLog()->debug("creating rpc endpoints");

    provideCore();
    provideAuthUpdate();

    serviceLog()->debug("rpc endpoints created");
}
