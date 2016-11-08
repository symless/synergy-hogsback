#include <multicast.hpp>
#include <lsif.hpp>
#include <iostream>

std::vector<MulticastInterface>
get_all_multicast_interfaces() {
    std::vector<MulticastInterface> ret;
    auto netdevices = get_all_netdevices();

    auto eth_begin = netdevices.lower_bound("en");
    auto eth_end = netdevices.upper_bound(std::string("en\xff", 3));
    for (auto i = eth_begin; i != eth_end; ++i) {
        MulticastInterface mifc;
        mifc.name = i->first;
        mifc.primary_ip = i->second.to_string();
        ret.push_back (std::move (mifc));
    }

    auto wifi_begin = netdevices.lower_bound("wl");
    auto wifi_end = netdevices.upper_bound(std::string("wl\xff", 3));
    for (auto i = wifi_begin; i != wifi_end; ++i) {
        MulticastInterface mifc;
        mifc.name = i->first;
        mifc.primary_ip = i->second.to_string();
        ret.push_back (std::move (mifc));
    }

    return ret;
}
