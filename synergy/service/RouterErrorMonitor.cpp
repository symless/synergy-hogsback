#include <synergy/service/RouterErrorMonitor.h>
#include <synergy/service/router/Router.hpp>
#include <synergy/service/ServiceLogs.h>
#include <synergy/common/ProfileConfig.h>

struct RouterErrorScreenMonitor {
    RouterErrorScreenMonitor (int64_t screenId, boost::asio::io_service&);
    ~RouterErrorScreenMonitor();

    void setReachable(bool reachable);
    void startTimer();
    void stopTimer();
    void notify();

    int64_t m_screenId;
    boost::asio::steady_timer m_timer;
    boost::signals2::signal<void(int64_t)> onReachable;
    boost::signals2::signal<void(int64_t)> onUnreachable;
    bool m_reachable = false;
    bool m_timerRunning = false;
};

RouterErrorMonitor::RouterErrorMonitor
(std::shared_ptr<ProfileConfig> localProfileConfig, Router& router) :
    m_router (router),
    m_localProfileConfig (localProfileConfig)
{
    m_localProfileConfig->screenOnline.connect ([this](Screen const& screen) {
        bool added = false;
        auto& screenMonitor = this->addScreen (screen.id(), added);

        if (added) {
            screenMonitor.startTimer();
        }
        else {
            if (!screenMonitor.m_timerRunning) {
                serviceLog()->debug ("Screen {} is now online", screen.id());
                screenMonitor.startTimer();
            }
        }
    });

    m_localProfileConfig->screenOffline.connect ([this](Screen const& screen) {
        auto monitor = std::find_if (begin(m_monitors), end(m_monitors),
                                     [screenId = screen.id()](auto& monitor){
           return (monitor->m_screenId == screenId);
        });

        if (monitor == end(m_monitors)) {
            return;
        }

        (*monitor)->stopTimer();
    });

    m_router.on_node_reachable.connect([this](int64_t const screenId) {
        bool added = false;
        auto& screenMonitor = this->addScreen(screenId, added);

        screenMonitor.setReachable(true);

        if (!added) {
            if (!screenMonitor.m_timerRunning) {
                serviceLog()->debug ("Screen {} is now reachable over the LAN", screenId);
                screenMonitor.notify();
            }
        }
    });

    m_router.on_node_unreachable.connect([this](int64_t const screenId) {

        auto monitor = std::find_if (begin(m_monitors), end(m_monitors),
                                     [screenId](auto& monitor){
           return (monitor->m_screenId == screenId);
        });

        if (monitor == end(m_monitors)) {
            return;
        }

        auto monitorPtr = monitor->get();
        monitorPtr->setReachable(false);
        if (!monitorPtr->m_timerRunning) {
            monitorPtr->startTimer();
        }
    });
}

RouterErrorMonitor::~RouterErrorMonitor() {
}

RouterErrorScreenMonitor&
RouterErrorMonitor::addScreen (int64_t screenId, bool& added)
{
    auto monitor = std::find_if (begin(m_monitors), end(m_monitors),
                                 [screenId](auto& monitor){
       return (monitor->m_screenId == screenId);
    });

    if (monitor == end(m_monitors)) {
        m_monitors.push_back (std::make_unique<RouterErrorScreenMonitor>
                              (screenId, m_router.getIoService()));

        auto& lastMonitor = m_monitors.back();

        lastMonitor->onReachable.connect ([this](int64_t const screenId) {
            this->screenReachable (screenId);
        });

        lastMonitor->onUnreachable.connect ([this](int64_t const screenId) {
            this->screenUnreachable (screenId);
        });
        added = true;
        return (*lastMonitor);
    }

    added = false;
    return (**monitor);
}

RouterErrorScreenMonitor::RouterErrorScreenMonitor (
    int64_t const screenId,
    asio::io_service& ioService):  m_screenId (screenId),
    m_timer (ioService) {
}

RouterErrorScreenMonitor::~RouterErrorScreenMonitor() {
}

void
RouterErrorScreenMonitor::setReachable(bool reachable)
{
    m_reachable = reachable;
}

void
RouterErrorScreenMonitor::startTimer()
{
    m_timer.expires_from_now (std::chrono::seconds(3));
    m_timerRunning = true;

    serviceLog()->debug ("Waiting for screen {} to become reachable", m_screenId);
    m_timer.async_wait ([this](auto ec) {
        m_timerRunning = false;
        if (ec == boost::asio::error::operation_aborted) {
            return;
        } else if (ec) {
            // TODO
        }

        serviceLog()->debug ("Screen {} reachable timer expired", m_screenId);
        this->notify();
    });
}

void
RouterErrorScreenMonitor::stopTimer()
{
    if (!m_timerRunning) {
        return;
    }
    m_timer.cancel();
    m_timer.get_io_service().poll();
}

void
RouterErrorScreenMonitor::notify()
{
    if (m_reachable) {
        serviceLog()->debug ("Screen {} became reachable", m_screenId);
        onReachable (m_screenId);
    } else {
        serviceLog()->debug ("Screen {} became unreachable", m_screenId);
        onUnreachable (m_screenId);
    }
}
