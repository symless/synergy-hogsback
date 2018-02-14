#include <synergy/config/lib/ServiceProxy.h>

#include <synergy/config/lib/ErrorView.h>
#include <synergy/common/WampClient.h>
#include "ScreenListModel.h"
#include "LogManager.h"
#include "ProcessMode.h"
#include "AppConfig.h"
#include "synergy/config/lib/CloudClient.h"

#include <boost/asio/deadline_timer.hpp>
#include <QtNetwork>
#include <QtGlobal>
#include <iostream>
#ifndef Q_OS_WIN
#include <unistd.h>
#endif

ServiceProxy::ServiceProxy() :
    m_wampClient(m_io, nullptr) // TODO
{
    connect (this, &ServiceProxy::rpcReceivedScreens, this,
             &ServiceProxy::onRpcReceivedScreens, Qt::QueuedConnection);

    connect (this, &ServiceProxy::logCoreOutput, this,
             &ServiceProxy::onLogCoreOutput, Qt::QueuedConnection);

    connect (this, &ServiceProxy::logServiceOutput, this,
             &ServiceProxy::onLogServiceOutput, Qt::QueuedConnection);

    connect (this, &ServiceProxy::rpcCloudOnline, this,
             &ServiceProxy::onRpcCloudOnline, Qt::QueuedConnection);

    connect (this, &ServiceProxy::rpcCloudOffline, this,
             &ServiceProxy::onRpcCloudOffline, Qt::QueuedConnection);

    connect (this, &ServiceProxy::rpcAuthLogout, this,
             &ServiceProxy::onRpcAuthLogout, Qt::QueuedConnection);

    connect (this, &ServiceProxy::rpcVersionCheck, this,
             &ServiceProxy::onRpcVersionCheck, Qt::QueuedConnection);

    m_wampClient.disconnected.connect([&]() {
        m_errorView->setMode(ErrorViewMode::kServiceError);
    });

    m_wampClient.connecting.connect([&]() {
        LogManager::debug("connecting to background service");
    });

    m_wampClient.connected.connect([&]() {
        LogManager::debug("connected to background service");

#if __linux__
        // for use on linux, tell the core process what user id it should run as.
        // this is a simple way to allow the core process to talk to X. this avoids
        // the "WARNING: primary screen unavailable: unable to open screen" error.
        // a better way would be to use xauth cookie and dbus to get access to X.
        std::string uid = std::to_string(getuid());
        LogManager::debug(QString("sending uid to service: %1").arg(uid.c_str()));
        m_wampClient.call<void>("synergy.core.set_uid", uid);
#endif

        m_wampClient.subscribe ("synergy.profile.snapshot", [&](std::string json) {
            emit rpcReceivedScreens(QString::fromStdString(json));
        });

        m_wampClient.subscribe ("synergy.core.log", [this](std::string line) {
            emit logCoreOutput(QString::fromStdString(line));
        });

        m_wampClient.subscribe ("synergy.service.log", [this](std::string line) {
            emit logServiceOutput(QString::fromStdString(line));
        });

        m_wampClient.subscribe ("synergy.screen.error",
                              [this](std::string screenName, int errorCode) {
            emit screenError(QString::fromStdString(screenName), errorCode);
        });

        m_wampClient.subscribe ("synergy.cloud.offline", [this]() {
            emit rpcCloudOffline();
        });

        m_wampClient.subscribe ("synergy.cloud.online", [this]() {
            emit rpcCloudOnline();
        });

        m_wampClient.subscribe ("synergy.auth.logout", [this]() {
            emit rpcAuthLogout();
        });

        m_wampClient.subscribe ("synergy.version.check", [this]() {
            emit rpcVersionCheck();
        });

        LogManager::debug("saying hello to background service");
        m_wampClient.call<void>("synergy.hello");
    });
}

ServiceProxy::~ServiceProxy()
{
}

void ServiceProxy::start()
{
    m_rpcThread = std::make_unique<std::thread>([this]{
        m_wampClient.start("127.0.0.1", 24888);
        m_io.run();
    });
}

void ServiceProxy::join()
{
    // stop rpc
    m_io.stop();
    m_rpcThread->join();
}

void
ServiceProxy::onRpcReceivedScreens(QString json)
{
    QByteArray byteArray(json.toUtf8());
    emit receivedScreens(byteArray);
}

void
ServiceProxy::onLogCoreOutput(QString text)
{
    foreach(QString line, text.split(QRegExp("\r|\n|\r\n"))) {
        if (!line.isEmpty()) {
            LogManager::raw("[ Core    ] " + line);
        }
    }
}

void ServiceProxy::onRpcCloudOffline()
{
    LogManager::debug(QString("service cloud connection error"));
    m_errorView->setMode(ErrorViewMode::kCloudError);
}

void ServiceProxy::onRpcCloudOnline()
{
    LogManager::debug(QString("service cloud connection recovered"));
    m_errorView->setMode(ErrorViewMode::kNone);
}

void ServiceProxy::onRpcAuthLogout()
{
    emit authLogout();
}

void ServiceProxy::onRpcVersionCheck()
{
    CloudClient* cloudClient = qobject_cast<CloudClient*>(CloudClient::instance());
    cloudClient->enableVersionCheck();
    cloudClient->checkUpdate();
}

std::shared_ptr<ErrorView>
ServiceProxy::errorView() const
{
    return m_errorView;
}

void
ServiceProxy::setErrorView(const std::shared_ptr<ErrorView> &errorView)
{
    m_errorView = errorView;

    QObject::connect(errorView.get(), &ErrorView::retryRequested, [&](ErrorViewMode mode){
        if (mode == ErrorViewMode::kCloudError) {
            m_wampClient.call<void>("synergy.cloud.retry");
        }
    });
}

void ServiceProxy::requestProfileSnapshot()
{
    if (m_wampClient.isConnected()) {
        m_wampClient.call<void>("synergy.snapshot.request");
    }
}

void ServiceProxy::serverClaim(int screenId)
{
    m_wampClient.call<void> ("synergy.server.claim",screenId);
}

WampClient&
ServiceProxy::wampClient()
{
    return m_wampClient;
}

void
ServiceProxy::onLogServiceOutput(QString text)
{
    foreach(QString line, text.split(QRegExp("\r|\n|\r\n"))) {
        if (!line.isEmpty()) {
            LogManager::raw(line);
        }
    }
}
