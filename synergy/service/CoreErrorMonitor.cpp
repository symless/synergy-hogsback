#include <synergy/service/CoreErrorMonitor.h>

#include <synergy/service/CoreProcess.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/join.hpp>

CoreErrorMonitor::CoreErrorMonitor(const std::string &localScreenName) :
    m_localScreenName(localScreenName)
{
}

void CoreErrorMonitor::monitor(CoreProcess& process)
{
    using boost::algorithm::contains;

    process.output.connect ([this](std::string const& line) {
        if (contains (line, "Address already in use")) {
            error(m_localScreenName, ScreenError::kCoreZombieProcess, "");
        }
    });

    process.output.connect ([this](std::string const& line) {
        static boost::regex const rgx ("duplicate screen name \"(.*)\"$");
        boost::match_results<std::string::const_iterator> results;
        if (!regex_search (line, results, rgx)) {
            return;
        }

        auto duplicateName = results[1].str();
        if (!duplicateName.empty()) {
            std::string message = "Found a duplicate screen: " + duplicateName + " in configure file.";
            error(m_localScreenName, ScreenError::kCoreDuplicateName, message);
        }
    });
}
