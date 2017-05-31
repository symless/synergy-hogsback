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

#include <QApplication>
#include <QMessageBox>
#include <QtQuick>
#include <QQmlApplicationEngine>
#include <stdexcept>

// TODO: Somehow get these in to a half decent <crashpad/...> form
#include <client/crashpad_client.h>
#include <client/crash_report_database.h>
#include <client/settings.h>

static auto const CRASHPAD_TOKEN =
    "27c61f07c3a6ac73851bb4d34e5615a9a6008f23c3b897acba7c0cee53f08bdc";

#ifdef Q_OS_DARWIN
#include <cstdlib>
#endif

void openAccessibilityDialog();

static bool
startCrashHandler()
{
    DirectoryManager directoryManager;
    auto db_path = directoryManager.crashDumpDir().toStdWString();
    auto handler_path = QDir(directoryManager.installedDir()
                         + "/crashpad_handler.exe").path().toStdWString();

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
        true  /* asynchronous_start */
    );
    if (!rc) {
        return false;
    }

    /* Wait for Crashpad to initialize. */
    rc = client.WaitForHandlerStart(INFINITE);
    if (!rc) {
        return false;
    }

    return true;
}

int main(int argc, char* argv[])
{
#ifdef Q_OS_DARWIN
    /* Workaround for QTBUG-40332
     * "High ping when QNetworkAccessManager is instantiated" */
    ::setenv ("QT_BEARER_POLL_TIMEOUT", "-1", 1);
#endif
    QCoreApplication::setOrganizationName ("Symless");
    QCoreApplication::setOrganizationDomain ("https://symless.com/");
    QCoreApplication::setApplicationName ("Synergy");

    /* This must be started after QApplication is constructed, because it
     * depends on file paths that are unavailable until then */
    QApplication app(argc, argv);
    startCrashHandler();

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
        qmlRegisterType<CloudClient>("com.synergy.gui", 1, 0, "CloudClient");
        qmlRegisterType<ConnectivityTester>("com.synergy.gui", 1, 0, "ConnectivityTester");
        qRegisterMetaType<Profile>();
        qmlRegisterType<Profile>("com.synergy.gui", 1, 0, "Profile");
        qmlRegisterType<ProfileListModel>("com.synergy.gui", 1, 0, "ProfileListModel");
        qmlRegisterSingletonType<AppConfig>("com.synergy.gui", 1, 0, "AppConfig", AppConfig::instance);
        qmlRegisterSingletonType<VersionManager>("com.synergy.gui", 1, 0, "VersionManager", VersionManager::instance);

        QQmlApplicationEngine engine;
        LogManager::instance();
        LogManager::setQmlContext(engine.rootContext());
        LogManager::info(QString("log filename: %1").arg(LogManager::logFilename()));

        engine.rootContext()->setContextProperty("PixelPerPoint", pixelPerPoint);
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

        char* volatile x = 0;
        *x = 'x';

        return app.exec();
    }
    catch (std::runtime_error& e) {
        LogManager::error(QString("exception caught: ").arg(e.what()));
        return 0;
    }
}
