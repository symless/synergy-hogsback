#include <QApplication>
#include <synergy/tray/Tray.h>
#include <synergy/common/DirectoryManager.h>
#include <synergy/common/CrashHandler.h>

int
main (int argc, char* argv[]) {
    try {
        DirectoryManager::instance()->init(argv[0]);
        startCrashHandler();
    } catch (...) {
    }

    QApplication app (argc, argv);
    Tray tray;

    if (!tray.waitUntilReadyFor (std::chrono::seconds (5))) {
        return EXIT_FAILURE;
    }

    tray.show();
    app.exec();
}
