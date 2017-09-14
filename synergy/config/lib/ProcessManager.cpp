#include "ProcessManager.h"

#include "ScreenListModel.h"
#include "LogManager.h"
#include "ProcessMode.h"
#include "AppConfig.h"
#include <synergy/common/WampClient.h>

#include <QtNetwork>
#include <QtGlobal>
#include <iostream>
#ifndef Q_OS_WIN
#include <unistd.h>
#endif

ProcessManager::ProcessManager() {
    /* QML registered types must be default constructible. This is a hack */
    throw std::runtime_error ("Default constructed process manager used");
}

ProcessManager::ProcessManager(WampClient& wampClient) :
    m_wampClient(std::shared_ptr<void>(), &wampClient)
{
    connect (this, &ProcessManager::rpcScreenStatusChanged, this,
             &ProcessManager::onRpcScreenStatusChanged, Qt::QueuedConnection);

    connect (this, &ProcessManager::logCoreOutput, this,
             &ProcessManager::onLogCoreOutput, Qt::QueuedConnection);

    connect (this, &ProcessManager::logServiceOutput, this,
             &ProcessManager::onLogServiceOutput, Qt::QueuedConnection);

    wampClient.connected.connect([&]() {
        wampClient.subscribe ("synergy.core.log", [this](std::string line) {
            emit logCoreOutput(QString::fromStdString(line));
        });

        wampClient.subscribe ("synergy.service.log", [this](std::string line) {
            emit logServiceOutput(QString::fromStdString(line));
        });

        wampClient.subscribe ("synergy.screen.status",
                              [this](std::string screenName, int status) {
            emit rpcScreenStatusChanged(QString::fromStdString(screenName), status);
        });

        wampClient.subscribe ("synergy.screen.error",
                              [this](std::string screenName, int errorCode) {
            emit screenError(QString::fromStdString(screenName), errorCode);
        });
    });
}

ProcessManager::~ProcessManager()
{
}

void
ProcessManager::onRpcScreenStatusChanged(QString name, int status)
{
    QPair<QString, ScreenStatus> r;
    r.first = name;
    r.second = (ScreenStatus)status;
    emit screenStatusChanged(r);
}

void ProcessManager::onLogCoreOutput(QString text)
{
    foreach(QString line, text.split(QRegExp("\r|\n|\r\n"))) {
        if (!line.isEmpty()) {
            LogManager::raw("[ Core    ] " + line);
        }
    }
}

void ProcessManager::onLogServiceOutput(QString text)
{
    foreach(QString line, text.split(QRegExp("\r|\n|\r\n"))) {
        if (!line.isEmpty()) {
            LogManager::raw("[ Service ] " + line);
        }
    }
}
