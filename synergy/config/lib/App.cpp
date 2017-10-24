#include "App.h"

#include <synergy/common/DirectoryManager.h>
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
#include <ProfileListModel.h>
#include <ProfileManager.h>

#include <QMessageBox>
#include <QtQuick>
#include <QQmlApplicationEngine>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <synergy/common/WampClient.h>
#include <synergy/common/CrashHandler.h>
#ifdef Q_OS_OSX
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace asio = boost::asio;
using namespace std;

#ifdef Q_OS_DARWIN
#include <cstdlib>
#endif

cxxopts::Options g_options("");

#ifndef Q_OS_OSX
static inline bool installServiceHelper() {
    return false;
}
#else
extern "C++" bool installServiceHelper();
extern bool iAmInstalled();
extern void killInstalledComponents();

static void
uninstallBundle() {
    boost::filesystem::path path ("/Applications/Synergy.app");
    std::cout << "Removing " << path << " ... ";
    boost::system::error_code ec;
    boost::filesystem::remove_all (path, ec);
    std::cout << (ec ? "Failed" : "OK") << "\n";
}

static void
reinstallBundle() {
    // Get a reference to the main bundle
    CFBundleRef mainBundle = CFBundleGetMainBundle();

    // Get a reference to the file's URL
    CFURLRef versionTxtUrl = CFBundleCopyResourceURL(mainBundle, CFSTR("Version"), CFSTR("txt"), NULL);
    if (!versionTxtUrl) {
        return;
    }

    // Convert the URL reference into a string reference
    CFStringRef imagePath = CFURLCopyFileSystemPath(versionTxtUrl, kCFURLPOSIXPathStyle);

    // Get the system encoding method
    CFStringEncoding encodingMethod = CFStringGetSystemEncoding();

    // Convert the string reference into a C string
    boost::filesystem::path path (CFStringGetCStringPtr(imagePath, encodingMethod));
    path = path.parent_path().parent_path().parent_path();
    path = boost::filesystem::canonical(path);

    uninstallBundle();
    std::cout << "Installing " << path << " to /Applications...\n";
}

void
App::installAndStartService()
{
    if (!iAmInstalled()) {
        stopService();
        killInstalledComponents();
        reinstallBundle();
        if (installServiceHelper()) {
            std::clog << "Service helper installed\n";
            sleep (3);
        }
    }

    startService();
}
#endif

int
App::run(int argc, char* argv[])
{
    std::vector<std::string> arguments(argv, argv + argc);

    cxxopts::Options options("synergy-config");

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

    installAndStartService();
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
        DirectoryManager::instance()->init(argv[0]);
        auto installDir = DirectoryManager::instance()->installDir().string();
        LogManager::debug(QString("install dir: %1").arg(installDir.c_str()));
    }
    catch (const std::exception& ex) {
        LogManager::error(QString("failed to init dirs: %1").arg(ex.what()));
        return EXIT_FAILURE;
    }
    catch (...) {
        LogManager::error("failed to init dirs: unknown error");
        return EXIT_FAILURE;
    }

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
    qmlRegisterType<LogManager>("com.synergy.gui", 1, 0, "LogManager");
    qmlRegisterSingletonType<CloudClient>("com.synergy.gui", 1, 0, "CloudClient", CloudClient::instance);
    qmlRegisterSingletonType<ProfileManager>("com.synergy.gui", 1, 0, "ProfileManager", ProfileManager::instance);
    qmlRegisterSingletonType<AppConfig>("com.synergy.gui", 1, 0, "AppConfig", AppConfig::instance);
    qmlRegisterSingletonType<VersionManager>("com.synergy.gui", 1, 0, "VersionManager", VersionManager::instance);

    QQmlApplicationEngine engine;
    LogManager* logManager = qobject_cast<LogManager*>(LogManager::instance());
    LogManager::setQmlContext(engine.rootContext());
    LogManager::info(QString("log filename: %1").arg(LogManager::logFilename()));

    ServiceProxy serviceProxy;

    auto errorView = std::make_shared<ErrorView>();
    serviceProxy.setErrorView(errorView);
    QObject::connect(errorView.get(), &ErrorView::retryRequested, [&](ErrorViewMode mode){
        if (mode == ErrorViewMode::kServiceError) {
            // restart the app when there's a service rpc connection error.
            restart(app, arguments);
        }
    });

    CloudClient* cloudClient = qobject_cast<CloudClient*>(CloudClient::instance());

    QObject::connect(cloudClient, &CloudClient::uploadLogFileSuccess, [&](QString url){
        logManager->setDialogUrl(url);
    });

    WampClient& wampClient = serviceProxy.wampClient();

    QObject::connect(cloudClient, &CloudClient::profileUpdated, [&wampClient](){
        // TODO: review this, this is essentially from a http call, probably should be in service
        auto authUpdateFunc = [&wampClient]() {
            AppConfig* appConfig = qobject_cast<AppConfig*>(AppConfig::instance());
            wampClient.call<void> ("synergy.auth.update",
                                   appConfig->userId(),
                                   appConfig->screenId(),
                                   appConfig->profileId(),
                                   appConfig->userToken().toStdString());
        };

        if (wampClient.isConnected()) {
            authUpdateFunc();
        }
        else {
            wampClient.connected.connect(authUpdateFunc);
        }
    });

    // service proxy start must happen after the cloud client connect
    // and wamp connect handler (above) has been connected
    serviceProxy.start();

    if (!g_options.count("disable-version-check")) {
        cloudClient->checkUpdate();
    }

    engine.rootContext()->setContextProperty
        ("kPixelPerPoint", QGuiApplication::primaryScreen()->physicalDotsPerInch() / 72);
    engine.rootContext()->setContextProperty
        ("qmlServiceProxy", static_cast<QObject*>(&serviceProxy));
    engine.rootContext()->setContextProperty
        ("qmlErrorView", static_cast<QObject*>(errorView.get()));
    engine.rootContext()->setContextProperty
        ("qmlLogManager", static_cast<QObject*>(logManager));

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    auto qtAppRet = app.exec();

    serviceProxy.join();

    return qtAppRet;
}

void
App::restart(QApplication& app, std::vector<std::string> argsVector)
{
    QString path = QString::fromStdString(argsVector.front());

    // for some reason app.arguments() only returns 1 arg.
    QStringList args;
    foreach (std::string argStr, argsVector) {
        QString arg = QString::fromStdString(argStr);
        if (arg != path) {
            args << arg;
        }
    }

    // HACK: quit and relaunch with same args
    LogManager::debug(QString("restarting app with: %1 %2").arg(path).arg(args.join(" ")));
    app.quit();
    QProcess::startDetached(path, args);
}

void
App::startService()
{
    QProcess serviceLoader;
    QString cmd("launchctl load /Library/LaunchAgents/com.symless.synergy.synergy-service.plist");
    serviceLoader.start(cmd);
    serviceLoader.waitForFinished(5000);
}

void
App::stopService()
{
    {
        QProcess serviceLoader;
        QString cmd("launchctl unload /Library/LaunchAgents/com.symless.synergy.v2.synergyd.plist");
        serviceLoader.start(cmd);
        serviceLoader.waitForFinished(5000);
    }
    {
        QProcess serviceLoader;
        QString cmd("launchctl unload /Library/LaunchAgents/com.symless.synergy.synergy-service.plist");
        serviceLoader.start(cmd);
        serviceLoader.waitForFinished(5000);
    }
}
