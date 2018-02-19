#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
#include <synergy/tray/Controls.h>
#include <mutex>
#include <condition_variable>

static std::mutex g_controlsMutex;
static bool g_controlsReady = false;
static std::condition_variable g_controlsReadyCV;

int
main (int argc, char* argv[]) {
    QApplication app (argc, argv);
    TrayControls trayControls;

    QIcon trayIcon (":/synergy/tray/icon.svg");
    QMenu trayMenu;
    QSystemTrayIcon tray;

    trayMenu.addAction ("Pause");
    trayMenu.addAction ("Restart");
    trayMenu.addSeparator ();
    trayMenu.addAction ("Quit", &app, &QApplication::quit);

    tray.setIcon(trayIcon);
    tray.setContextMenu (&trayMenu);
    tray.show();

    trayControls.ready.connect ([&]() {
        g_controlsMutex.lock();
        g_controlsReady = true;
        g_controlsMutex.unlock();
        g_controlsReadyCV.notify_all();
    });

    trayControls.connect();

    std::unique_lock<std::mutex> lock (g_controlsMutex);
    if (!g_controlsReadyCV.wait_for (lock, std::chrono::seconds(5), [&]() {
        return g_controlsReady;
    })) {
        return EXIT_FAILURE;
    }

    app.exec();
}

