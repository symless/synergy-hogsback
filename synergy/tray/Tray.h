#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
#include <synergy/tray/Controls.h>
#include <mutex>
#include <condition_variable>

class Tray : public QObject
{
    Q_OBJECT

public:
    Tray();

    void
    show();

    template <typename Duration>
    bool
    waitUntilReadyFor (Duration&& dur);

signals:
    void coreDisabled(bool);

public slots:
    void onCoreDisabled(bool disabled);

private:
    TrayControls m_controls;
    std::mutex m_controlsMutex;
    std::condition_variable m_controlsReadyCV;
    bool m_controlsReady = false;

    QIcon m_svg;
    QSystemTrayIcon m_icon;
    QMenu m_menu;
    QAction* m_disableAction = nullptr;
    QAction* m_enableAction = nullptr;
};

template <typename Duration> inline
bool
Tray::waitUntilReadyFor (Duration&& dur) {
    std::unique_lock<std::mutex> lock (m_controlsMutex);

    return m_controlsReadyCV.wait_for (lock, dur, [&]() {
        return m_controlsReady;
    });
}
