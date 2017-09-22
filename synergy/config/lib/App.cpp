#include "App.h"

#include <synergy/config/lib/ServiceProxy.h>
#include <synergy/config/lib/ErrorView.h>
#include "CloudClient.h"
#include "ScreenListModel.h"
#include "ScreenManager.h"
#include "FontManager.h"
#include "LogManager.h"
#include "AccessibilityManager.h"
#include "VersionManager.h"
#include "AppConfig.h"
#include "Hostname.h"
#include "Common.h"
#include <synergy/common/DirectoryManager.h>
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
using namespace std;

#ifdef Q_OS_DARWIN
#include <cstdlib>
#endif

cxxopts::Options g_options("");

App::App(bool (*installServiceHelper)())
{
    m_installServiceHelper = installServiceHelper;
}

#ifdef Q_OS_OSX

bool
App::installService() {
    if (!boost::filesystem::exists
            ("/Library/LaunchDaemons/com.symless.synergy.v2.ServiceHelper.plist")) {
        std::clog << "Service helper not installed, installing...\n";
        if (!m_installServiceHelper()) {
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
    // cache the directory of this binary for installedDir
    boost::filesystem::path selfPath = boost::filesystem::system_complete(argv[0]);
    DirectoryManager::instance()->m_programDir = selfPath.remove_filename().string();
    // TODO: figure out linux linker error and move boost path stuff back to init
    //DirectoryManager::instance()->init(argc, argv);

    cxxopts::Options options("synergy2");

    options.add_options()
      ("help", "Print command line argument help")
      ("disable-version-check", "Disable version check")
      ("use-test-cloud", "Connect to the test cloud server")
#ifdef Q_OS_OSX
      ("service", "Start the service (Mac only)")
#endif
    ;

    options.parse(argc, argv);

    g_options = options;

    if (g_options.count("help"))
    {
      std::cout << g_options.help({"", "Group"}) << std::endl;
      return 0;
    }

#ifdef Q_OS_OSX

    if (g_options.count("service")) {
        QProcess service;
        QString cmd("/Applications/Synergy.app/Contents/MacOS/synergy-service");
        QStringList args;
        service.start(cmd, args);
        service.waitForFinished(-1);
        return 0;
    }

    if (installService()) {
        QProcess serviceLoader;
        QString cmd("launchctl load /Library/LaunchAgents/com.symless.synergy.synergy-service.plist");
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

    try {
        startCrashHandler();
    }
    catch (const std::exception& ex) {
        LogManager::error(QString("failed to start crash handler: %1").arg(ex.what()));
    }
    catch (...) {
        LogManager::error(QString("failed to start crash handler: unknown error"));
    }

    FontManager::loadAll();

    qmlRegisterType<ErrorView>("com.synergy.gui", 1, 0, "ErrorView");
    qmlRegisterType<Hostname>("com.synergy.gui", 1, 0, "Hostname");
    qmlRegisterType<ScreenListModel>("com.synergy.gui", 1, 0, "ScreenListModel");
    qmlRegisterType<ScreenManager>("com.synergy.gui", 1, 0, "ScreenManager");
    qmlRegisterType<ServiceProxy>("com.synergy.gui", 1, 0, "ServiceProxy");
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

    auto errorView = std::make_shared<ErrorView>();

    ServiceProxy serviceProxy;
    serviceProxy.setErrorView(errorView);
    serviceProxy.start();

    CloudClient* cloudClient = qobject_cast<CloudClient*>(CloudClient::instance());
    WampClient& wampClient = serviceProxy.wampClient();

    QObject::connect(cloudClient, &CloudClient::profileUpdated, [&wampClient](){
        AppConfig* appConfig = qobject_cast<AppConfig*>(AppConfig::instance());
        wampClient.call<void> ("synergy.auth.update",
                               appConfig->userId(),
                               appConfig->screenId(),
                               appConfig->profileId(),
                               appConfig->userToken().toStdString());

        LogManager::debug("requesting profile snapshot");
        wampClient.call<void> ("synergy.profile.request");
    });

    if (!g_options.count("disable-version-check")) {
        cloudClient->checkUpdate();
    }

    engine.rootContext()->setContextProperty
        ("kPixelPerPoint", QGuiApplication::primaryScreen()->physicalDotsPerInch() / 72);
    engine.rootContext()->setContextProperty
        ("qmlServiceProxy", static_cast<QObject*>(&serviceProxy));
    engine.rootContext()->setContextProperty
        ("qmlErrorView", static_cast<QObject*>(errorView.get()));

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    auto qtAppRet = app.exec();

    serviceProxy.join();

    return qtAppRet;
}
