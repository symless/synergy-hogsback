#include "ConnectivityTester.h"
#include "CloudClient.h"
#include "ScreenListModel.h"
#include "ScreenManager.h"
#include "TrialValidator.h"
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


#if (defined(Q_OS_WIN) || defined (Q_OS_DARWIN)) && !defined (QT_DEBUG)
// TODO: Somehow get these in to a half decent <crashpad/...> form
#include <client/crashpad_client.h>
#include <client/crash_report_database.h>
#include <client/settings.h>

static auto const CRASHPAD_TOKEN =
    "cc4db6ef2d4731b276b0a111979336443dc0372624effb79d8b29b26a200e1c6";
#endif

#ifdef Q_OS_DARWIN
#include <cstdlib>
#endif

void openAccessibilityDialog();

static bool
startCrashHandler()
{
#if (defined(Q_OS_WIN) || defined (Q_OS_DARWIN)) && !defined (QT_DEBUG)
    DirectoryManager directoryManager;

#if defined(Q_OS_WIN)
    auto db_path = directoryManager.crashDumpDir().toStdWString();
    auto handler_path = QDir(directoryManager.installedDir()
                         + "/crashpad_handler.exe").path().toStdWString();
#elif defined(Q_OS_DARWIN)
    auto db_path = directoryManager.crashDumpDir().toStdString();
    auto handler_path = QDir(directoryManager.installedDir()
                         + "/crashpad_handler").path().toStdString();
#endif

    using namespace crashpad;
    base::FilePath db (db_path);
    base::FilePath handler (handler_path);

    /* Enable uploads */
    {
        auto crashReportDatabase = crashpad::CrashReportDatabase::Initialize(db);
        crashReportDatabase->GetSettings()->SetUploadsEnabled(true);
    }

    CrashpadClient client;
    bool rc = false;

    std::string url("https://synergy.sp.backtrace.io:6098");
    std::map<std::string, std::string> annotations;
    annotations["token"] = CRASHPAD_TOKEN;
    annotations["format"] = "minidump";

    std::vector<std::string> arguments;
    arguments.push_back ("--no-rate-limit");

    rc = client.StartHandler (handler, db, db, url, annotations, arguments,
        true, /* restartable */
        false  /* asynchronous_start */
    );
    if (!rc) {
        return false;
    }

#if defined(Q_OS_WIN)
    /* Wait for Crashpad to initialize. */
    rc = client.WaitForHandlerStart(INFINITE);
    if (!rc) {
        return false;
    }
#endif
#endif
    return true;
}

#ifdef Q_OS_OSX
bool installServiceHelper();

static void
checkService() {
    if (!boost::filesystem::exists
            ("/Library/LaunchDaemons/com.symless.synergy.v2.ServiceHelper.plist")) {
        std::clog << "Service helper not installed, installing...\n";
        if (!installServiceHelper()) {
            std::clog << "Failed to install service helper" << "\n";
            return;
        }
        std::clog << "Service helper installed\n";
    }
}
#endif

int
main(int argc, char* argv[])
{
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

    /* This must be started after QApplication is constructed, because it
     * depends on file paths that are unavailable until then */
    QApplication app(argc, argv);
    startCrashHandler();

#ifdef Q_OS_OSX
    //checkService();
#endif

    TrialValidator trialValidator;
    if (!trialValidator.isValid()) {
        QMessageBox msgBox;
        msgBox.setText("This version is not supported anymore. Please <a href='https://www.symless.com'>download</a> the latest build.");
        msgBox.exec();
        return 0;
    }

    FontManager::loadAll();

    qreal dpi = QGuiApplication::primaryScreen()->physicalDotsPerInch();
    // 72 points = 1 inch
    qreal pixelPerPoint = dpi / 72;

    try {
        qmlRegisterType<Hostname>("com.synergy.gui", 1, 0, "Hostname");
        qmlRegisterType<ScreenListModel>("com.synergy.gui", 1, 0, "ScreenListModel");
        qmlRegisterType<ScreenManager>("com.synergy.gui", 1, 0, "ScreenManager");
        qmlRegisterType<ProcessManager>("com.synergy.gui", 1, 0, "ProcessManager");
        qmlRegisterType<AccessibilityManager>("com.synergy.gui", 1, 0, "AccessibilityManager");
        qmlRegisterType<ConnectivityTester>("com.synergy.gui", 1, 0, "ConnectivityTester");
        qmlRegisterType<ProfileListModel>("com.synergy.gui", 1, 0, "ProfileListModel");
        qmlRegisterSingletonType<CloudClient>("com.synergy.gui", 1, 0, "CloudClient", CloudClient::instance);
        qmlRegisterSingletonType<ProfileManager>("com.synergy.gui", 1, 0, "ProfileManager", ProfileManager::instance);
        qmlRegisterSingletonType<AppConfig>("com.synergy.gui", 1, 0, "AppConfig", AppConfig::instance);
        qmlRegisterSingletonType<VersionManager>("com.synergy.gui", 1, 0, "VersionManager", VersionManager::instance);
        qmlRegisterSingletonType<VersionManager>("com.synergy.gui", 1, 0, "LogManager", LogManager::instance);

        QQmlApplicationEngine engine;
        LogManager::instance();
        LogManager::setQmlContext(engine.rootContext());
        LogManager::info(QString("log filename: %1").arg(LogManager::logFilename()));

        engine.rootContext()->setContextProperty("PixelPerPoint", pixelPerPoint);
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
        return app.exec();
    }
    catch (std::runtime_error& e) {
        LogManager::error(QString("exception caught: ").arg(e.what()));
        return 0;
    }
}
