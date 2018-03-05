#include "lsif.hpp"
#include <vector>
#include <cstring>
#include <cassert>
#include <codecvt>
#include <Iphlpapi.h>

std::map<std::string, boost::asio::ip::address>
get_all_netdevices () {
    std::map<std::string, boost::asio::ip::address> map;

    ULONG size = 16 * 1024; // 16 KB recommended preallocation
    std::vector<IP_ADAPTER_ADDRESSES> adapters;
    ULONG res;

    do {
        adapters.resize (size / sizeof(IP_ADAPTER_ADDRESSES));
        size = adapters.size() * sizeof(IP_ADAPTER_ADDRESSES);
        res = GetAdaptersAddresses(
            AF_INET,
            GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_MULTICAST |
                GAA_FLAG_SKIP_ANYCAST,
            NULL,
            adapters.data(),
            &size
        );
    } while (res == ERROR_BUFFER_OVERFLOW);

    if (res != ERROR_SUCCESS) {
        throw std::runtime_error ("Failed to get a list of network interfaces");
    }

    adapters.resize (size / sizeof(IP_ADAPTER_ADDRESSES));
    auto adapter = adapters.data();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;

    do {
        auto ip = adapter->FirstUnicastAddress;
        if (!ip) {
            continue;
        }
        auto name = convert.to_bytes (adapter->FriendlyName);
        do {
            if (ip->Address.iSockaddrLength < sizeof(SOCKADDR_IN)) {
                throw std::runtime_error ("Failed to get interface IP address");
            }
            auto& addr_in = *reinterpret_cast<SOCKADDR_IN*>
                                (ip->Address.lpSockaddr);
            auto& addr = addr_in.sin_addr;

            boost::asio::ip::address_v4::bytes_type bytes;
            static_assert (sizeof(addr) == sizeof(bytes), "");
            std::memcpy (&bytes, &addr, sizeof(bytes));

            map.emplace (std::move (name), boost::asio::ip::address_v4(bytes));

        } while (ip = ip->Next);
    } while (adapter = adapter->Next);

    return map;
}

#ifdef LSIF_APP
#include <iostream>

int
main (int, char**) {
    auto map = get_all_netdevices();
    for (auto& e : map) {
        std::cout << e.first << " -> " << e.second << std::endl;
    }
}
#endif
