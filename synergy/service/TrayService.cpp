#include <synergy/service/TrayService.h>
#include <synergy/service/ServiceLogs.h>

static auto const kTrayPingTimeout = std::chrono::seconds (3);

bool
TrayService::start() {
    if (m_trayRunning) {
        return false;
    }

    m_trayRunning = true;
    this->ping();
    return true;
}

void
TrayService::stop() {
    boost::system::error_code ec;
    m_pingTimer.cancel(ec);
    m_trayRunning = false;
}

void
TrayService::ping() {
    m_pingTimer.expires_from_now (kTrayPingTimeout);
    m_pingTimer.async_wait ([this](auto const ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        } else if (ec) {
            serviceLog()->critical ("Tray service monitor failed.");
            return;
        }
        m_trayRunning = false;
    });
}
