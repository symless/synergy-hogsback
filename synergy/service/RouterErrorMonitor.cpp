#include <synergy/service/RouterErrorMonitor.h>

#include <synergy/service/router/Router.hpp>
#include <synergy/service/ServiceLogs.h>
#include <synergy/common/ProfileConfig.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>

RouterErrorMonitor::RouterErrorMonitor(std::shared_ptr<ProfileConfig> localProfileConfig) :
    m_localProfileConfig (localProfileConfig)
{
    m_localProfileConfig->screenOnline.connect([this](Screen screen){
        std::vector<std::string> ipList;
        std::string ipListStr = screen.ipList();

        boost::split(ipList, ipListStr, boost::is_any_of(","));

        for(const auto& ipStr : ipList) {
            add(screen.id(), ipStr);
        }
    });

    m_localProfileConfig->screenOffline.connect([this](Screen screen){
        remove(screen.id());
    });
}

void RouterErrorMonitor::monitor(Router& router)
{
    // TODO: implement
}

void RouterErrorMonitor::add(int64_t screenId, std::string Ip)
{
    auto ret = m_monitoringScreenIp.equal_range(screenId);

    for (auto it = ret.first; it != ret.second; ++it) {
        if (it->second == Ip) {
            return;
        }
    }

    m_monitoringScreenIp.insert(std::pair<int64_t, std::string>(screenId, Ip));
}

void RouterErrorMonitor::remove(int64_t screenId)
{
    m_monitoringScreenIp.erase(screenId);
}
