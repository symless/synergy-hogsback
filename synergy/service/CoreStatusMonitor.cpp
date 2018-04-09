#include <synergy/service/CoreStatusMonitor.h>

#include <synergy/service/CoreProcess.h>
#include <synergy/common/Hostname.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/join.hpp>

CoreStatusMonitor::CoreStatusMonitor(std::shared_ptr<UserConfig> userConfig, std::shared_ptr<ProfileConfig> localProfileConfig) :
    m_userConfig(userConfig),
    m_localProfileConfig(localProfileConfig)
{
}

void CoreStatusMonitor::monitor(CoreProcess& process)
{
    reset();

    using boost::algorithm::contains;
    int localScreenId = m_userConfig->screenId();

    if (process.processMode() == ProcessMode::kClient) {
        m_signals.emplace_back (
            process.output.connect ([this, localScreenId](std::string const& line) {
                if (!contains (line, "connected to server")) {
                    return;
                }

                update(localScreenId, ScreenStatus::kConnected);
            }, boost::signals2::at_front)
        );

        m_signals.emplace_back (
            process.output.connect ([this, localScreenId](std::string const& line) {
                if (!contains (line, "connecting to")) {
                    return;
                }

                update(localScreenId, ScreenStatus::kConnecting);
            }, boost::signals2::at_front)
        );

        m_signals.emplace_back (
            process.output.connect ([this, localScreenId](std::string const& line) {
                if (!contains (line, "disconnected from server")) {
                    return;
                }

                update(localScreenId, ScreenStatus::kDisconnected);
            }, boost::signals2::at_front)
        );
    }
    else if (process.processMode() == ProcessMode::kServer) {
        m_signals.emplace_back (
            process.output.connect ( [this, localScreenId] (std::string const& line) {
                if (!contains (line, "started server, waiting for clients")) {
                    return;
                }

                update(localScreenId, ScreenStatus::kConnected);
            }, boost::signals2::at_front)
        );
    }
}

void CoreStatusMonitor::update(const int screenId, ScreenStatus status)
{
    auto originalStatus = m_screenStates[screenId];
    m_screenStates[screenId] = status;

    if (status != originalStatus) {
        screenStatusChanged(screenId, status);
    }
}

void CoreStatusMonitor::reset()
{
    /* Disconnect internal signal handling */
    for (auto& signal: m_signals) {
        signal.disconnect();
    }

    m_signals.clear();
}
