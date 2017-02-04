#include "lsif.hpp"
#include "unix_util.hpp"
#include <boost/endian/conversion.hpp>
#include <boost/scope_exit.hpp>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/types.h>

std::map<std::string, boost::asio::ip::address>
get_all_netdevices () {
    std::map<std::string, boost::asio::ip::address> map;

    struct ::ifaddrs* ifa = nullptr;
    if (getifaddrs (&ifa)) {
        throw_errno ();
    }

    BOOST_SCOPE_EXIT (ifa) {
        freeifaddrs (ifa);
    }
    BOOST_SCOPE_EXIT_END

    do {
        if (!ifa->ifa_addr || (ifa->ifa_addr->sa_family != AF_INET)) {
            continue;
        }
        auto& addr = reinterpret_cast<struct sockaddr_in&> (*ifa->ifa_addr);
        map.emplace (ifa->ifa_name,
                     boost::asio::ip::address_v4 (
                         boost::endian::big_to_native (addr.sin_addr.s_addr)));
    } while ((ifa = ifa->ifa_next));

    return map;
}


#ifdef LSIF_APP
#include <iostream>

int
main (int, char**) {
    auto map = get_all_netdevices ();
    for (auto& e : map) {
        std::cout << e.first << " -> " << e.second << std::endl;
    }
}
#endif
