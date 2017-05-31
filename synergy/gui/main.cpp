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
#include <ProfileListModel.h>

#include <QApplication>
#include <QMessageBox>
#include <QtQuick>
#include <QQmlApplicationEngine>
#include <stdexcept>

#ifdef Q_OS_DARWIN
#include <cstdlib>
#endif

void openAccessibilityDialog();

int main(int argc, char* argv[])
{
#ifdef Q_OS_DARWIN
    /* Workaround for QTBUG-40332 - "High ping when QNetworkAccessManager is instantiated" */
    ::setenv ("QT_BEARER_POLL_TIMEOUT", "-1", 1);
#endif

    QCoreApplication::setOrganizationName("Symless");
    QCoreApplication::setOrganizationDomain("http://symless.com/");
    QCoreApplication::setApplicationName("Synergy v2");

    QApplication app(argc, argv);
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
        LogManager::error(QString("exception catched: ")
                            .arg(e.what()));
        return 0;
    }
}
