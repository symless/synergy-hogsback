#include <boost/asio/ip/address.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/scope_exit.hpp>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <net/if.h>
#include <netinet/in.h>
#include <string>
#include <synergy/service/lsif.hpp>
#include <synergy/service/unix_util.hpp>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

template <typename T>
inline auto
byte_size_of (std::vector<T> const& v) noexcept {
    return v.size () * sizeof (T);
}

NetworkInterfaceMap
get_netdevice_map () {
    NetworkInterfaceMap map;

    int fd = ::socket (AF_INET, SOCK_DGRAM, 0);
    BOOST_SCOPE_EXIT (&fd) {
        ::close (fd);
        fd = -1;
    }
    BOOST_SCOPE_EXIT_END

    std::vector<struct ifreq> interfaces;
    struct ::ifconf conf;
    std::size_t ifc_size, ifc_ret;
    std::size_t const num_tries = 100;
    auto tries_left             = num_tries;

    do {
        /* Limit the number of attempts we try to grab network interfaces */
        if (!tries_left--) {
            throw std::runtime_error (
                "Failed to get a complete list of network interfaces");
        }

        /* Count the number of interfaces on the machine (null buffer) */
        std::memset (&conf, 0, sizeof (conf));
        if (::ioctl (fd, SIOCGIFCONF, &conf)) {
            throw_errno ();
        }

        /* Create a buffer of twice what we need (but a minimum of 16) */
        ifc_size = boost::numeric_cast<std::size_t> (conf.ifc_len);
        assert ((ifc_size % sizeof (struct ifreq)) == 0);
        ifc_size = 2 * std::max (size_t (8), ifc_size / sizeof (struct ifreq));
        interfaces.resize (ifc_size);

        /* Get the list of interfaces in to the new buffer */
        conf.ifc_len = boost::numeric_cast<int> (byte_size_of (interfaces));
        conf.ifc_req = interfaces.data ();
        if (::ioctl (fd, SIOCGIFCONF, &conf)) {
            throw_errno ();
        }

        /* Count the number of interfaces returned  */
        ifc_ret = boost::numeric_cast<std::size_t> (conf.ifc_len);
        assert ((ifc_ret % sizeof (struct ifreq)) == 0);
        ifc_ret /= sizeof (struct ifreq);

        /* Loop until we get a buffer that wasn't filled */
    } while (ifc_ret == ifc_size);

    /* Resize the buffer to the number of interfaces */
    assert (ifc_ret < ifc_size);
    interfaces.resize (ifc_ret);

    for (auto& ifc : interfaces) {
        if (ifc.ifr_addr.sa_family != AF_INET) {
            continue;
        }
        auto& addr = reinterpret_cast<struct sockaddr_in&> (ifc.ifr_addr);
        map.insert (NetworkInterfaceMap::value_type (
            ifc.ifr_name,
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
