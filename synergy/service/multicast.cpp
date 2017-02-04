#include <synergy/service/multicast.hpp>
#include <synergy/service/lsif.hpp>

std::vector<MulticastInterface>
get_all_multicast_interfaces () {
    std::vector<MulticastInterface> ret;
    auto netdevices = get_all_netdevices ();

    auto eth_begin = netdevices.lower_bound ("en");
    auto eth_end   = netdevices.lower_bound (std::string ("eo", 2));
    for (auto i = eth_begin; i != eth_end; ++i) {
        MulticastInterface mifc;
        mifc.name       = i->first;
        mifc.primary_ip = i->second.to_string ();
        ret.push_back (std::move (mifc));
    }

    auto wifi_begin = netdevices.lower_bound ("wl");
    auto wifi_end   = netdevices.lower_bound (std::string ("wm", 2));
    for (auto i = wifi_begin; i != wifi_end; ++i) {
        MulticastInterface mifc;
        mifc.name       = i->first;
        mifc.primary_ip = i->second.to_string ();
        ret.push_back (std::move (mifc));
    }

    return ret;
}
