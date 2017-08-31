#include "App.h"
#include "ConnectivityTester.h"
#include "CloudClient.h"
#include "ScreenListModel.h"
#include "ScreenManager.h"
#include "FontManager.h"
#include "LogManager.h"
#include "ProcessManager.h"
#include "AccessibilityManager.h"
#include "VersionManager.h"
#include "AppConfig.h"
#include "Hostname.h"
#include "Common.h"
#include <DirectoryManager.h>
#include <ProfileListModel.h>
#include <ProfileManager.h>
#include <QApplication>
#include <QMessageBox>
#include <QtQuick>
#include <QQmlApplicationEngine>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <synergy/common/WampClient.h>
#include <synergy/common/CrashHandler.h>

namespace asio = boost::asio;

#ifdef Q_OS_DARWIN
#include <cstdlib>
#endif

App::App() :
    m_options("Synergy Config", "Makes it super easy to configure Synergy")
{
    m_options.add_options()
      ("disable-version-check", "Disable version check")
#ifdef Q_OS_OSX
      ("service", "Start the service (Mac only)")
#endif
    ;
}

#ifdef Q_OS_OSX
bool installServiceHelper();

static bool
installService() {
    if (!boost::filesystem::exists
            ("/Library/LaunchDaemons/com.symless.synergy.v2.ServiceHelper.plist")) {
        std::clog << "Service helper not installed, installing...\n";
        if (!installServiceHelper()) {
            std::clog << "Failed to install service helper" << "\n";
            return false;
        }
        std::clog << "Service helper installed\n";
        sleep(3);
        return true;
    }
    return false;
}
#endif

int
App::run(int argc, char* argv[])
{
    m_options.parse(argc, argv);

#ifdef Q_OS_OSX

    if (!m_options.count("service")) {
        QProcess service;
        QString cmd("/Applications/Synergy.app/Contents/MacOS/synergyd");
        QStringList args;
        service.start(cmd, args);
        service.waitForFinished(-1);
        return 0;
    }

    if (installService()) {
        QProcess serviceLoader;
        QString cmd("launchctl load /Library/LaunchAgents/com.symless.synergy.v2.synergyd.plist");
        serviceLoader.start(cmd);
        serviceLoader.waitForFinished(5000);
    }

#endif

    /* Workaround for QTBUG-40332
     * "High ping when QNetworkAccessManager is instantiated" */
#if defined (Q_OS_WIN)
    _putenv_s ("QT_BEARER_POLL_TIMEOUT", "-1");
#else
    setenv ("QT_BEARER_POLL_TIMEOUT", "-1", 1);
#endif
    QCoreApplication::setOrganizationName ("Symless");
    QCoreApplication::setOrganizationDomain ("https://symless.com/");
    QCoreApplication::setApplicationName ("Synergy");

    /* The crash handler must be started after QApplication is constructed,
     * because it depends on file paths that are unavailable until then. */
    QApplication app(argc, argv);
    startCrashHandler();

    FontManager::loadAll();

    asio::io_service io;
    QObject::connect (&app, &QCoreApplication::aboutToQuit,
        [rpcWork = std::make_shared<asio::io_service::work> (io)]() mutable {
            rpcWork.reset();
        }
    );

    qmlRegisterType<Hostname>("com.synergy.gui", 1, 0, "Hostname");
    qmlRegisterType<ScreenListModel>("com.synergy.gui", 1, 0, "ScreenListModel");
    qmlRegisterType<ScreenManager>("com.synergy.gui", 1, 0, "ScreenManager");
    qmlRegisterType<ProcessManager>("com.synergy.gui", 1, 0, "ProcessManager");
    qmlRegisterType<AccessibilityManager>("com.synergy.gui", 1, 0, "AccessibilityManager");
    qmlRegisterType<ProfileListModel>("com.synergy.gui", 1, 0, "ProfileListModel");
    qmlRegisterSingletonType<CloudClient>("com.synergy.gui", 1, 0, "CloudClient", CloudClient::instance);
    qmlRegisterSingletonType<ProfileManager>("com.synergy.gui", 1, 0, "ProfileManager", ProfileManager::instance);
    qmlRegisterSingletonType<AppConfig>("com.synergy.gui", 1, 0, "AppConfig", AppConfig::instance);
    qmlRegisterSingletonType<VersionManager>("com.synergy.gui", 1, 0, "VersionManager", VersionManager::instance);
    qmlRegisterSingletonType<LogManager>("com.synergy.gui", 1, 0, "LogManager", LogManager::instance);

    QQmlApplicationEngine engine;
    LogManager::instance();
    LogManager::setQmlContext(engine.rootContext());
    LogManager::info(QString("log filename: %1").arg(LogManager::logFilename()));

    // TODO: refactor main function
    WampClient wampClient (io);

    CloudClient* cloudClient = qobject_cast<CloudClient*>(CloudClient::instance());

    QObject::connect(cloudClient, &CloudClient::profileUpdated, [&wampClient](){
        AppConfig* appConfig = qobject_cast<AppConfig*>(AppConfig::instance());
        wampClient.call<void> ("synergy.auth.update",
                               appConfig->userId(),
                               appConfig->screenId(),
                               appConfig->profileId(),
                               appConfig->userToken().toStdString());
    });

    ProcessManager processManager (wampClient);
    ConnectivityTester tester;
    processManager.setConnectivityTester(&tester);

    wampClient.connected.connect([&]() {
        wampClient.subscribe ("synergy.profile.snapshot", [&](std::string json) {
            QByteArray byteArray(json.c_str(), json.length());
            cloudClient->receivedScreensInterface(byteArray);
        });
    });

    std::thread rpcThread ([&]{
        wampClient.start ("127.0.0.1", 24888);
        io.run ();
    });

    if (!m_options.count("disable-version-check")) {
        cloudClient->checkUpdate();
    }

    engine.rootContext()->setContextProperty
        ("PixelPerPoint", QGuiApplication::primaryScreen()->physicalDotsPerInch() / 72);
    engine.rootContext()->setContextProperty
        ("rpcProcessManager", static_cast<QObject*>(&processManager));

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    auto qtAppRet = app.exec();

    /* The RPC thread should stop and join us when it runs out of work */
    rpcThread.join();

    return qtAppRet;
}
