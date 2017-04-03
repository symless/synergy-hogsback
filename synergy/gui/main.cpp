#include "ConnectivityTester.h"
#include "CloudClient.h"
#include "ScreenListModel.h"
#include "ScreenManager.h"
#include "LogManager.h"
#include "ProcessManager.h"
#include "AppConfig.h"
#include "Hostname.h"
#include "Common.h"

#include <QApplication>
#include <QtQuick>
#include <QQmlApplicationEngine>
#include <stdexcept>

void openAccessibilityDialog();

int main(int argc, char* argv[])
{
    QCoreApplication::setOrganizationName("Symless");
    QCoreApplication::setOrganizationDomain("http://symless.com/");
    QCoreApplication::setApplicationName("Synergy v2");

    QApplication app(argc, argv);

    openAccessibilityDialog();

    try {
        qmlRegisterType<Hostname>("com.synergy.gui", 1, 0, "Hostname");
        qmlRegisterType<ScreenListModel>("com.synergy.gui", 1, 0, "ScreenListModel");
        qmlRegisterType<ScreenManager>("com.synergy.gui", 1, 0, "ScreenManager");
        qmlRegisterType<ProcessManager>("com.synergy.gui", 1, 0, "ProcessManager");
        qmlRegisterType<CloudClient>("com.synergy.gui", 1, 0, "CloudClient");
        qmlRegisterType<ConnectivityTester>("com.synergy.gui", 1, 0, "ConnectivityTester");
        qmlRegisterSingletonType<AppConfig>("com.synergy.gui", 1, 0, "AppConfig", AppConfig::instance);

        QQmlApplicationEngine engine;
        LogManager::instance();
        LogManager::setQmlContext(engine.rootContext());
        LogManager::info(QString("log filename: %1").arg(LogManager::logFilename()));
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")))
                ;
        QIcon icon(":res/image/synergy-icon.png");
        app.setWindowIcon(icon);

        return app.exec();
    }
    catch (std::runtime_error& e) {
        LogManager::error(QString("exception catched: ")
                            .arg(e.what()));
        return 0;
    }
}
