#include "AppConfig.h"
#include "Hostname.h"
#include "CloudClient.h"
#include "ScreenListModel.h"
#include "ScreenManager.h"
#include "LogManager.h"
#include "ProcessManager.h"
#include "Common.h"

#include <QApplication>
#include <QtQuick>
#include <QQmlApplicationEngine>
#include <QtWebEngine/qtwebengineglobal.h>
#include <stdexcept>

int main(int argc, char* argv[])
{
    QCoreApplication::setOrganizationName("Symless");
    QCoreApplication::setOrganizationDomain("http://symless.com/");
    QCoreApplication::setApplicationName("Synergy v2");

    QApplication app(argc, argv);
    LogManager::instance();
    QtWebEngine::initialize();

    try {
        qmlRegisterType<Hostname>("com.synergy.gui", 1, 0, "Hostname");
        qmlRegisterType<ScreenListModel>("com.synergy.gui", 1, 0, "ScreenListModel");
        qmlRegisterType<ScreenManager>("com.synergy.gui", 1, 0, "ScreenManager");
        qmlRegisterType<ProcessManager>("com.synergy.gui", 1, 0, "ProcessManager");
        qmlRegisterType<CloudClient>("com.synergy.gui", 1, 0, "CloudClient");
        qmlRegisterSingletonType<AppConfig>("com.synergy.gui", 1, 0, "AppConfig", AppConfig::instance);
        QQmlApplicationEngine engine(QUrl(QStringLiteral("qrc:/main.qml")));

        QIcon icon(":res/image/synergy.ico");
        app.setWindowIcon(icon);

        return app.exec();
    }
    catch (std::runtime_error& e) {
        LogManager::error(QString("exception catched: ")
                            .arg(e.what()));
        return 0;
    }
}
