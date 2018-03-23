#include <synergy/service/IPMonitor.h>
#include <synergy/service/NetworkAdapters.h>
#include <boost/asio/spawn.hpp>

static auto kIPMonitorInterval = std::chrono::seconds(3);

IPMonitor::IPMonitor
(boost::asio::io_service& ioService): m_pollTimer(ioService) {
}

void
IPMonitor::start() {
    if (m_running) {
        return;
    }

    m_running = true;

    boost::asio::spawn (m_pollTimer.get_io_service(), [this](auto ctx) {
        while (true) {
            if (!this->m_running) {
                return;
            }

            boost::system::error_code ec;
            this->m_pollTimer.expires_from_now (kIPMonitorInterval);
            this->m_pollTimer.async_wait (ctx[ec]);

            if (ec) {
                this->m_running = false;
                if (ec == boost::asio::error::operation_aborted) {
                    return;
                }
                throw boost::system::system_error (ec, ec.message());
            }

            std::set<boost::asio::ip::address> ipAddresses;
            auto adapterIPAddresses = getAdapterIPAddresses();
            for (auto& adapter: adapterIPAddresses) {
                ipAddresses.insert (std::move (adapter.second));
            }

            if (ipAddresses != m_knownIPAddresses) {
                m_knownIPAddresses = std::move (ipAddresses);
                this->ipSetChanged (m_knownIPAddresses);
            }
        }
    });
}

void
IPMonitor::stop() {
    if (!m_running) {
        return;
    }
    m_pollTimer.cancel();
}
