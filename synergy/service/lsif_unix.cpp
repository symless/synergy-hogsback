#include <boost/endian/conversion.hpp>
#include <boost/scope_exit.hpp>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <synergy/service/lsif.hpp>
#include <synergy/service/unix_util.hpp>
#include <sys/types.h>

NetworkInterfaceMap
get_netdevice_map () {
    NetworkInterfaceMap map;

    struct ::ifaddrs* ifa = nullptr;
    if (getifaddrs (&ifa)) {
        throw_errno ();
    }

    BOOST_SCOPE_EXIT (ifa) {
        freeifaddrs (ifa);
    }
    BOOST_SCOPE_EXIT_END

    for (auto if_p = ifa; if_p != NULL; if_p = if_p->ifa_next) {
        if (!if_p->ifa_addr || (if_p->ifa_addr->sa_family != AF_INET)) {
            continue;
        }
        auto& addr = reinterpret_cast<struct sockaddr_in&> (*if_p->ifa_addr);
        map.insert (NetworkInterfaceMap::value_type (
            if_p->ifa_name,
            boost::asio::ip::address_v4 (
                boost::endian::big_to_native (addr.sin_addr.s_addr))));
    }

    return map;
}


#ifdef LSIF_APP
#include <iostream>

int
main (int, char**) {
    auto map = get_netdevice_map ();
    for (auto& e : map.left) {
        std::cout << e.first << " -> " << e.second << std::endl;
    }
}
#endif
