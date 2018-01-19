#include <synergy/service/CoreStatusMonitor.h>

#include <synergy/service/CoreProcess.h>

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
    Screen& localScreen = m_localProfileConfig->getScreen(m_userConfig->screenId());
    std::string localScreenName = localScreen.name();

    if (process.processMode() == ProcessMode::kClient) {
        m_signals.emplace_back (
            process.output.connect ([this, localScreenName](std::string const& line) {
                if (!contains (line, "connected to server")) {
                    return;
                }

                update(localScreenName, ScreenStatus::kConnected);
            }, boost::signals2::at_front)
        );

        m_signals.emplace_back (
            process.output.connect ([this, localScreenName](std::string const& line) {
                if (!contains (line, "connecting to")) {
                    return;
                }

                update(localScreenName, ScreenStatus::kConnecting);
            }, boost::signals2::at_front)
        );

        m_signals.emplace_back (
            process.output.connect ([this, &process](std::string const& line) {
                if (!contains (line, "server is dead")) {
                    return;
                }

                auto currentServerId = process.currentServerId();
                Screen& screen = m_localProfileConfig->getScreen(currentServerId);
                update(screen.name(), ScreenStatus::kDisconnected);
            }, boost::signals2::at_front)
        );
    }
    else if (process.processMode() == ProcessMode::kServer) {
        m_signals.emplace_back (
            process.output.connect ( [this, localScreenName] (std::string const& line) {
                if (!contains (line, "started server, waiting for clients")) {
                    return;
                }

                update(localScreenName, ScreenStatus::kConnected);
            }, boost::signals2::at_front)
        );

        m_signals.emplace_back (
            process.output.connect ([this](std::string const& line) {
                static boost::regex const rgx ("client \"(.*)\" has disconnected$");
                boost::match_results<std::string::const_iterator> results;
                if (!regex_search (line, results, rgx)) {
                    return;
                }

                auto clientScreenName = results[1].str();

                update(clientScreenName, ScreenStatus::kDisconnected);
            }, boost::signals2::at_front)
        );

        m_signals.emplace_back (
            process.output.connect ([this](std::string const& line) {
                static boost::regex const rgx ("client \"(.*)\" is dead$");
                boost::match_results<std::string::const_iterator> results;
                if (!regex_search (line, results, rgx)) {
                    return;
                }

                auto clientScreenName = results[1].str();

                update(clientScreenName, ScreenStatus::kDisconnected);
            }, boost::signals2::at_front)
        );
    }
}

void CoreStatusMonitor::update(const std::string &screenName, ScreenStatus status)
{
    auto originalStatus = m_screenStates[screenName];
    m_screenStates[screenName] = status;

    if (status != originalStatus) {
        screenStatusChanged(screenName, status);
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
