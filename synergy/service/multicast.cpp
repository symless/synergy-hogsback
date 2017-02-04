#include <synergy/service/lsif.hpp>
#include <synergy/service/multicast.hpp>

std::vector<MulticastInterface>
get_all_multicast_interfaces () {
    std::vector<MulticastInterface> ret;
    auto netdevices = get_netdevice_map ();

    auto eth_begin = netdevices.left.lower_bound ("en");
    auto eth_end   = netdevices.left.lower_bound (std::string ("eo", 2));
    for (auto i = eth_begin; i != eth_end; ++i) {
        MulticastInterface mifc;
        mifc.name       = i->first;
        mifc.primary_ip = i->second.to_string ();
        ret.push_back (std::move (mifc));
    }

    auto wifi_begin = netdevices.left.lower_bound ("wl");
    auto wifi_end   = netdevices.left.lower_bound (std::string ("wm", 2));
    for (auto i = wifi_begin; i != wifi_end; ++i) {
        MulticastInterface mifc;
        mifc.name       = i->first;
        mifc.primary_ip = i->second.to_string ();
        ret.push_back (std::move (mifc));
    }

    return ret;
}
