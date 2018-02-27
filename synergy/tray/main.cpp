#include <QApplication>
#include <synergy/tray/Tray.h>

int
main (int argc, char* argv[]) {
    QApplication app (argc, argv);
    Tray tray;

    if (!tray.waitUntilReadyFor (std::chrono::seconds (5))) {
        return EXIT_FAILURE;
    }

    tray.show();
    app.exec();
}
