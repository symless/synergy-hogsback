#include <synergy/service/RouterErrorMonitor.h>
#include <synergy/service/router/Router.hpp>
#include <synergy/service/ServiceLogs.h>
#include <synergy/common/ProfileConfig.h>

struct RouterErrorScreenMonitor {
    RouterErrorScreenMonitor (int64_t screenId, boost::asio::io_service&);
    ~RouterErrorScreenMonitor();

    int64_t m_screenId;
    boost::asio::steady_timer m_timer;
    boost::signals2::signal<void(int64_t)> onReachable;
    boost::signals2::signal<void(int64_t)> onUnreachable;
    bool m_reachable = false;

    void setReachable();
    void setUnreachable();
};

RouterErrorMonitor::RouterErrorMonitor
(std::shared_ptr<ProfileConfig> localProfileConfig, Router& router) :
    m_router (router),
    m_localProfileConfig (localProfileConfig)
{
    m_localProfileConfig->screenSetChanged.connect
    ([this](std::vector<Screen> addedScreens,
            std::vector<Screen> removedScreens) {

        /* Start monitoring added screens */
        for (auto& screen: addedScreens) {
            m_monitors.push_back (std::make_unique<RouterErrorScreenMonitor>
                                  (screen.id(), m_router.getIoService()));
            auto& monitor = m_monitors.back();

            monitor->onReachable.connect ([this](int64_t const screenId) {
                this->screenReachable (screenId);
            });

            monitor->onUnreachable.connect ([this](int64_t const screenId) {
                this->screenUnreachable (screenId);
            });
        }

        /* Stop monitoring removed screens */
        for (auto& screen: removedScreens) {
            m_monitors.erase (std::remove_if (begin(m_monitors), end(m_monitors),
                                [&](auto& monitor){
                return (monitor->m_screenId == screen.id());
            }), end(m_monitors));
        }
    });

    m_router.on_node_reachable.connect([this](int64_t const screenId) {
        auto monitor = std::find_if (begin(m_monitors), end(m_monitors),
                                     [screenId](auto& monitor){
           return (monitor->m_screenId == screenId);
        });
        if (monitor == end(m_monitors)) {
            return;
        }

        (*monitor)->setReachable();
    });

    m_router.on_node_unreachable.connect([this](int64_t const screenId) {
        auto monitor = std::find_if (begin(m_monitors), end(m_monitors),
                                     [screenId](auto& monitor){
           return (monitor->m_screenId == screenId);
        });
        if (monitor == end(m_monitors)) {
            return;
        }

        (*monitor)->setUnreachable();
    });
}

RouterErrorMonitor::~RouterErrorMonitor() {
}

RouterErrorScreenMonitor::RouterErrorScreenMonitor
(int64_t const screenId, asio::io_service& ioService):
    m_screenId (screenId),
    m_timer (ioService) {
    setUnreachable();
}

RouterErrorScreenMonitor::~RouterErrorScreenMonitor() {
}

void RouterErrorScreenMonitor::setReachable()
{
    m_reachable = true;
    m_timer.cancel();
    m_timer.get_io_service().poll();
    this->onReachable (m_screenId);
}

void RouterErrorScreenMonitor::setUnreachable()
{
    m_reachable = false;
    m_timer.expires_from_now (std::chrono::seconds(3));
    m_timer.async_wait ([this](auto ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        } else if (ec) {
            // TODO
        }

        assert (!m_reachable);
        this->onUnreachable (m_screenId);
    });
}
