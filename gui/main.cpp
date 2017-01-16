#include "Hostname.h"
#include "ScreenListModel.h"
#include "ScreenManager.h"
#include "LogManager.h"
#include "ProcessManager.h"
#include "Common.h"

#include <QApplication>
#include <QtQuick>
#include <QQmlApplicationEngine>
#include <stdexcept>

#if defined(Q_OS_MAC)
#include <Carbon/Carbon.h>
#endif

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    LogManager::instance();

#if defined(Q_OS_MAC)

    // new in mavericks, applications are trusted individually
    // with use of the accessibility api. this call will show a
    // prompt which can show the security/privacy/accessibility
    // tab, with a list of allowed applications. synergy should
    // show up there automatically, but will be unchecked.

    if (AXIsProcessTrusted()) {
        return true;
    }

    const void* keys[] = { kAXTrustedCheckOptionPrompt };
    const void* trueValue[] = { kCFBooleanTrue };
    CFDictionaryRef options = CFDictionaryCreate(NULL, keys, trueValue, 1, NULL, NULL);

    bool result = AXIsProcessTrustedWithOptions(options);
    CFRelease(options);

#endif

    try {
        qmlRegisterType<Hostname>("com.synergy.gui", 1, 0, "Hostname");
        qmlRegisterType<ScreenListModel>("com.synergy.gui", 1, 0, "ScreenListModel");
        qmlRegisterType<ScreenManager>("com.synergy.gui", 1, 0, "ScreenManager");
        qmlRegisterType<ProcessManager>("com.synergy.gui", 1, 0, "ProcessManager");
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
