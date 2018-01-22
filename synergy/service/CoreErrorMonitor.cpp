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

}
