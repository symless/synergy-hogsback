#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
#include <synergy/tray/Controls.h>

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

    app.exec();
    trayControls.log()->debug ("Tray process exited");
}

