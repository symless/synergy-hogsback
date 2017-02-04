#include "lsif.hpp"
#include <vector>
#include <cstring>
#include <cassert>
#include <codecvt>
#include <Iphlpapi.h>

std::map<std::string, boost::asio::ip::address>
get_all_netdevices () {
    std::map<std::string, boost::asio::ip::address> ret;

    ULONG size = 16 * 1024;
    std::vector<IP_ADAPTER_ADDRESSES> adapters;
    ULONG res;

    do {
        adapters.resize (size / sizeof(IP_ADAPTER_ADDRESSES));
        size = adapters.size() * sizeof(IP_ADAPTER_ADDRESSES);
        res = GetAdaptersAddresses(
            AF_INET,
            GAA_FLAG_SKIP_DNS_SERVER,
            NULL,
            adapters.data(),
            &size
        );
    } while (res == ERROR_BUFFER_OVERFLOW);

    if (res != ERROR_SUCCESS) {
        return ret;
    }

    adapters.resize (size / sizeof(IP_ADAPTER_ADDRESSES));
    auto adapter = adapters.data();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
    do {
        auto name = convert.to_bytes(adapter->FriendlyName);
        auto ip = adapter->FirstUnicastAddress;
        if (!ip) {
            continue;
        }
        do {
            assert(ip->Address.iSockaddrLength >= sizeof(SOCKADDR_IN));
            auto& addr_in = *reinterpret_cast<SOCKADDR_IN*> (ip->Address.lpSockaddr);
            auto& addr = addr_in.sin_addr;
            boost::asio::ip::address_v4::bytes_type bytes;
            assert(sizeof(addr) == sizeof(bytes));
            std::memcpy(&bytes, &addr, sizeof(bytes));
            ret.emplace (name, boost::asio::ip::address_v4(bytes));
        } while (ip = ip->Next);
    } while (adapter = adapter->Next);

    return ret;
}

#ifdef LSIF_APP
#include <iostream>

int
main(int, char**) {
    auto map = get_all_netdevices();
    for (auto& e : map) {
        std::cout << e.first << " -> " << e.second << std::endl;
    }
}
#endif