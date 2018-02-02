#include <synergy/service/RouterErrorMonitor.h>
#include <synergy/service/router/Router.hpp>
#include <synergy/service/ServiceLogs.h>
#include <synergy/common/ProfileConfig.h>

struct RouterErrorScreenMonitor {
    explicit RouterErrorScreenMonitor (int64_t screenId,
                                       boost::asio::io_service&);
    int64_t m_screenId;
    boost::asio::steady_timer m_timer;
    bool m_reachable = false;
};

RouterErrorMonitor::RouterErrorMonitor
(std::shared_ptr<ProfileConfig> localProfileConfig, Router& router) :
    m_localProfileConfig (localProfileConfig)
{
    m_localProfileConfig->screenSetChanged.connect
    ([this](std::vector<Screen> addedScreens,
            std::vector<Screen> removedScreens) {

        /* Stop monitoring removed screens */
        for (auto& removed: removedScreens) {
            m_monitors.erase (std::remove_if (begin(m_monitors), end(m_monitors),
                            [&](auto& monitor){
                return (monitor->m_screenId == removed.id());
            }), end(m_monitors));
        }
    });
}

RouterErrorMonitor::~RouterErrorMonitor()
{
}

RouterErrorScreenMonitor::RouterErrorScreenMonitor
(int64_t const screenId, asio::io_service& ioService):
    m_screenId(screenId),
    m_timer (ioService) {
}
