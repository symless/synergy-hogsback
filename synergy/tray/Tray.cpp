#include <synergy/tray/Tray.h>

Tray::Tray():
    m_svg (":/synergy/tray/icon.svg")
{
    m_enableAction = m_menu.addAction ("Enable Synergy Service", this, SLOT(enableCore()));
    m_disableAction = m_menu.addAction ("Disable Synergy Service", this, SLOT(disableCore()));

    m_menu.addSeparator ();
    m_menu.addAction ("Quit Synergy Helper", QApplication::instance(), SLOT(quit()));

    m_icon.setIcon(m_svg);
    m_icon.setContextMenu (&m_menu);

    m_controls.ready.connect ([&]() {
        std::unique_lock<std::mutex> lock (m_controlsMutex);
        m_controlsReady = true;
        lock.unlock();
        m_controlsReadyCV.notify_all();
    });

    m_controls.statusChanged.connect ([&](bool disabled) {
        emit coreDisabled(disabled);
    });

    connect (this, &Tray::coreDisabled, this,
             &Tray::onCoreDisabled, Qt::QueuedConnection);

    m_controls.connect();
}

void
Tray::show() {
    m_icon.show();
}

void
Tray::enableCore() {
    this->m_controls.resume();
}

void
Tray::disableCore() {
    this->m_controls.pause();
}

void
Tray::onCoreDisabled(bool disabled)
{
    if (disabled) {
        m_disableAction->setEnabled(false);
        m_enableAction->setEnabled(true);
    }
    else {
        m_disableAction->setEnabled(true);
        m_enableAction->setEnabled(false);
    }

    m_menu.update();
    m_menu.repaint();
}
