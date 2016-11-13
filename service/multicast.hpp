#ifndef MULTICAST_HPP
#define MULTICAST_HPP

#include <string>
#include <vector>
#include <tuple>

static auto const MULTICAST_ADDR_FMT = "epgm://{};224.1.1.1:{}";

struct MulticastInterface {
    std::string name;
    std::string primary_ip;
};

inline bool
operator== (MulticastInterface const& ifc1, MulticastInterface const& ifc2) {
    return std::tie (ifc1.name, ifc1.primary_ip) ==
           std::tie (ifc2.name, ifc2.primary_ip);
}

inline bool
operator< (MulticastInterface const& ifc1, MulticastInterface const& ifc2) {
    return std::tie (ifc1.name, ifc1.primary_ip) <
           std::tie (ifc2.name, ifc2.primary_ip);
}

std::vector<MulticastInterface>
get_all_multicast_interfaces ();

#endif // MULTICAST_HPP
