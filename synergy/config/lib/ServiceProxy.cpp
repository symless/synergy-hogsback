#include <synergy/config/lib/ServiceProxy.h>

#include <synergy/config/lib/ErrorView.h>
#include <synergy/common/WampClient.h>
#include "ScreenListModel.h"
#include "LogManager.h"
#include "ProcessMode.h"
#include "AppConfig.h"

#include <boost/asio/deadline_timer.hpp>
#include <QtNetwork>
#include <QtGlobal>
#include <iostream>
#ifndef Q_OS_WIN
#include <unistd.h>
#endif

ServiceProxy::ServiceProxy() :
    m_wampClient(m_io)
{
    connect (this, &ServiceProxy::rpcScreenStatusChanged, this,
             &ServiceProxy::onRpcScreenStatusChanged, Qt::QueuedConnection);

    connect (this, &ServiceProxy::logCoreOutput, this,
             &ServiceProxy::onLogCoreOutput, Qt::QueuedConnection);

    connect (this, &ServiceProxy::logServiceOutput, this,
             &ServiceProxy::onLogServiceOutput, Qt::QueuedConnection);


    m_wampClient.connectionError.connect([&]() {
        m_errorView->setMode(ErrorViewMode::kServiceError);
    });

    m_wampClient.connecting.connect([&]() {
        LogManager::debug(QString("connecting to service"));
    });

    m_wampClient.connected.connect([&]() {
        LogManager::debug(QString("connected to service"));

        m_wampClient.subscribe ("synergy.profile.snapshot", [&](std::string json) {
            QByteArray byteArray(json.c_str(), json.length());
            emit receivedScreens(byteArray);
        });

        m_wampClient.subscribe ("synergy.core.log", [this](std::string line) {
            emit logCoreOutput(QString::fromStdString(line));
        });

        m_wampClient.subscribe ("synergy.service.log", [this](std::string line) {
            emit logServiceOutput(QString::fromStdString(line));
        });

        m_wampClient.subscribe ("synergy.screen.status",
                              [this](std::string screenName, int status) {
            emit rpcScreenStatusChanged(QString::fromStdString(screenName), status);
        });

        m_wampClient.subscribe ("synergy.screen.error",
                              [this](std::string screenName, int errorCode) {
            emit screenError(QString::fromStdString(screenName), errorCode);
        });

        m_wampClient.subscribe ("synergy.cloud.offline", [this](int retryTimeout) {
            LogManager::debug(QString("service cloud connection error"));
            m_errorView->setRetryTimeout(retryTimeout);
            m_errorView->setMode(ErrorViewMode::kCloudError);
        });

        m_wampClient.subscribe ("synergy.cloud.online", [this]() {
            LogManager::debug(QString("service cloud connection recovered"));
            m_errorView->setMode(ErrorViewMode::kNone);
        });

        LogManager::debug("requesting profile snapshot");
        m_wampClient.call<void> ("synergy.profile.request");
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
ServiceProxy::onRpcScreenStatusChanged(QString name, int status)
{
    QPair<QString, ScreenStatus> r;
    r.first = name;
    r.second = (ScreenStatus)status;
    emit screenStatusChanged(r);
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

std::shared_ptr<ErrorView>
ServiceProxy::errorView() const
{
    return m_errorView;
}

void
ServiceProxy::setErrorView(const std::shared_ptr<ErrorView> &errorView)
{
    m_errorView = errorView;
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
            LogManager::raw("[ Service ] " + line);
        }
    }
}
