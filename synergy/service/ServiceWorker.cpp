#include "ServiceWorker.h"

#include "synergy/service/ProfileSnapshot.h"
#include "synergy/service/ConnectivityTester.h"
#include "synergy/service/CloudClient.h"
#include <synergy/service/Logs.h>
#include <synergy/common/UserConfig.h>
#include <synergy/common/RpcManager.h>
#include <synergy/common/WampServer.h>
#include <synergy/common/WampRouter.h>
#include <synergy/common/ScreenStatus.h>
#include "ProcessManager.h"
#include <boost/asio.hpp>
#include <iostream>

std::string g_lastProfileSnapshot;

ServiceWorker::ServiceWorker(boost::asio::io_service& ioService) :
    m_ioService (ioService),
    m_work (std::make_shared<boost::asio::io_service::work>(ioService)),
    m_rpcManager (std::make_unique<RpcManager>(m_ioService)),
    m_processManager (std::make_unique<ProcessManager>(m_ioService)),
    m_connectivityTester (std::make_unique<ConnectivityTester>(m_ioService)),
    m_userConfig(std::make_shared<UserConfig>()),
    m_cloudClient (std::make_unique<CloudClient>(ioService, m_userConfig))
{
    m_userConfig->load();
    m_cloudClient->init();

    g_log.onLogLine.connect([this](std::string logLine) {
        auto server = m_rpcManager->server();
        server->publish ("synergy.service.log", std::move(logLine));
    });

    g_log.onLogLine.connect([this](std::string logLine) {
        auto server = m_rpcManager->server();
        server->publish ("synergy.service.log", std::move(logLine));
    });

    m_cloudClient->websocketMessageReceived.connect([this](std::string json){
        // parse message
        ProfileSnapshot snapshot;
        try {
            snapshot.parseJsonSnapshot(json);
        }
        catch (...) {
            // TODO: log error
            return;
        }

        // notify connectivity tester
        m_connectivityTester->testNewScreens(snapshot.getScreens());

        // forward the message via rpc server
        auto server = m_rpcManager->server();
        g_lastProfileSnapshot = json;
        server->publish ("synergy.profile.snapshot", std::move(json));
    });

    m_rpcManager->ready.connect([this]() {
        provideRpcEndpoints();
        mainLog()->info("service started successfully");
    });

    setupCloudClientCalls();

    m_rpcManager->ready.connect([this]() { provideRpcEndpoints(); });

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
                     [server](std::vector<std::string>& cmd) {
        mainLog()->debug("sending last profile snapshot");
        server->publish ("synergy.profile.snapshot", g_lastProfileSnapshot);
    });

    m_processManager->onOutput.connect(
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
        [server](std::string const& screenName, ErrorCode ec) {
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
        m_cloudClient->init();
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

void ServiceWorker::setupCloudClientCalls()
{
    m_connectivityTester->newReportGenerated.connect(
                [this](int screenId, std::string successfulIp, std::string failedIp){
        m_cloudClient->report(screenId, successfulIp, failedIp);
    });

    m_processManager->localInputDetected.connect([this](){
        m_cloudClient->claimServer();
    });
}

void ServiceWorker::provideRpcEndpoints()
{
    mainLog()->debug("creating rpc endpoints");

    provideCore();
    provideAuthUpdate();

    mainLog()->debug("rpc endpoints created");
}
