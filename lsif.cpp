#include "lsif.hpp"
#include <boost/asio/ip/address.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/scope_exit.hpp>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <net/if.h>
#include <netinet/in.h>
#include <stropts.h> // ioctl
#include <sys/ioctl.h>
#include <sys/socket.h> // socket
#include <sys/types.h>
#include <unistd.h> // close
#include <unordered_map>
#include <vector>

namespace sys = boost::system;

template <typename T>
inline auto
byte_size_of (std::vector<T> const& v) noexcept {
    return v.size () * sizeof (T);
}

__attribute__ ((noinline, noreturn)) static void
throw_errno () {
    throw sys::system_error (errno, sys::system_category ());
}

std::map<std::string, boost::asio::ip::address>
get_all_netdevices (std::size_t const num_tries) {
    std::map<std::string, boost::asio::ip::address> map;

    int fd = ::socket (AF_INET, SOCK_DGRAM, 0);
    BOOST_SCOPE_EXIT (&fd) {
        ::close (fd);
        fd = -1;
    }
    BOOST_SCOPE_EXIT_END

    std::vector<struct ifreq> interfaces;
    struct ::ifconf conf;
    std::size_t if_buf, if_ret;
    auto tries_left = num_tries;

    do {
        /* Limit the number of attempts we try to grab all network interfaces */
        if (!tries_left--) {
            throw std::runtime_error (
                "Failed to get list of network interfaces");
        }

        /* Count the number of interfaces on the machine (null buffer) */
        std::memset (&conf, 0, sizeof (conf));
        if (::ioctl (fd, SIOCGIFCONF, &conf)) {
            throw_errno ();
        }

        /* Create a buffer of twice what we need (minimum of 16) */
        if_buf = boost::numeric_cast<std::size_t> (conf.ifc_len);
        assert ((if_buf % sizeof (struct ifreq)) == 0);
        if_buf = 2 * std::max (size_t (8), if_buf / sizeof (struct ifreq));
        interfaces.resize (if_buf);

        /* Get the list of interfaces in to the new buffer */
        conf.ifc_len = boost::numeric_cast<int> (byte_size_of (interfaces));
        conf.ifc_req = interfaces.data ();
        if (::ioctl (fd, SIOCGIFCONF, &conf)) {
            throw_errno ();
        }

        /* Count the number of interfaces returned  */
        if_ret = boost::numeric_cast<std::size_t> (conf.ifc_len);
        assert ((if_ret % sizeof (struct ifreq)) == 0);
        if_ret /= sizeof (struct ifreq);

        /* Loop until we get a buffer that wasn't filled */
    } while (if_ret == if_buf);

    /* Resize the buffer to the number actually grabbed */
    interfaces.resize (if_ret);

    for (auto& ifc : interfaces) {
        if (ifc.ifr_addr.sa_family != AF_INET) {
            continue;
        }
        auto& addr = reinterpret_cast<struct sockaddr_in&> (ifc.ifr_addr);
        map.emplace (ifc.ifr_name,
                     boost::asio::ip::address_v4 (
                         boost::endian::big_to_native (addr.sin_addr.s_addr)));
    }

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
