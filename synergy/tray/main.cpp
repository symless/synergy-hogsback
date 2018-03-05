#include <QApplication>
#include <synergy/tray/Tray.h>
#include <synergy/common/DirectoryManager.h>
#include <synergy/common/CrashHandler.h>

#ifdef Q_OS_OSX
extern "C++" void hideDockIcon();
#endif

int
main (int argc, char* argv[]) {
    try {
        DirectoryManager::instance()->init(argv[0]);
        startCrashHandler();
    } catch (...) {
    }

#ifdef Q_OS_OSX
    // Hack: manually specify plugin directory
    // reason: because we move tray process into resources folder to stop
    // OS thinking it as an instance of UI, tray can't seem to find the
    // plugin
    // suggestion: use a separate bundle for tray so it would have it's own
    // plist and plugins
    QCoreApplication::addLibraryPath("/Applications/Synergy.app/Contents/PlugIns");
#endif

    QApplication app (argc, argv);
    Tray tray;

#ifdef Q_OS_OSX
    hideDockIcon();
#endif

    if (!tray.waitUntilReadyFor (std::chrono::seconds (5))) {
        return EXIT_FAILURE;
    }

    tray.show();
    app.exec();
}
