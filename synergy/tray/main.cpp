#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
#include <synergy/tray/Controls.h>
#include <mutex>
#include <condition_variable>

class Tray {
public:
    Tray();

    void
    show();

    template <typename Duration>
    bool
    waitUntilReadyFor (Duration&& dur);

private:
    TrayControls m_controls;
    std::mutex m_controlsMutex;
    std::condition_variable m_controlsReadyCV;
    bool m_controlsReady = false;

    QIcon m_svg;
    QSystemTrayIcon m_icon;
    QMenu m_menu;
    QAction* m_pauseAction = nullptr;
    QAction* m_resumeAction = nullptr;
};

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


Tray::Tray():
    m_svg (":/synergy/tray/icon.svg")
{
    m_pauseAction = m_menu.addAction ("Pause");
    m_menu.addSeparator ();
    m_menu.addAction ("Quit", QApplication::instance(), &QApplication::quit);

    m_icon.setIcon(m_svg);
    m_icon.setContextMenu (&m_menu);

    m_controls.ready.connect ([&]() {
        std::unique_lock<std::mutex> lock (m_controlsMutex);
        m_controlsReady = true;
        lock.unlock();
        m_controlsReadyCV.notify_all();
    });

    m_controls.connect();
}

void
Tray::show() {
    m_icon.show();
}

template <typename Duration> inline
bool
Tray::waitUntilReadyFor (Duration&& dur) {
    std::unique_lock<std::mutex> lock (m_controlsMutex);

    return m_controlsReadyCV.wait_for (lock, dur, [&]() {
        return m_controlsReady;
    });
}
