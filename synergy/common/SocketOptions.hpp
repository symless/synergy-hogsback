#pragma once
#include <boost/asio/ip/tcp.hpp>

#ifdef _WIN32
#include <Mstcpip.h>
#include <cstring>
#endif

#if defined(__APPLE__) || defined(__linux__)
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <fcntl.h>

#ifdef __APPLE__
using tcp_keep_alive_idle =
    boost::asio::detail::socket_option::integer<BOOST_ASIO_OS_DEF (IPPROTO_TCP),
                                                TCP_KEEPALIVE>;

#elif __linux__
using tcp_keep_alive_idle =
    boost::asio::detail::socket_option::integer<BOOST_ASIO_OS_DEF (IPPROTO_TCP),
                                                TCP_KEEPIDLE>;

#endif

using tcp_keep_alive_count =
    boost::asio::detail::socket_option::integer<BOOST_ASIO_OS_DEF (IPPROTO_TCP),
                                                TCP_KEEPCNT>;

using tcp_keep_alive_interval =
    boost::asio::detail::socket_option::integer<BOOST_ASIO_OS_DEF (IPPROTO_TCP),
                                                TCP_KEEPINTVL>;
#endif

inline bool
set_tcp_keep_alive_options (boost::asio::ip::tcp::socket& socket, int const idle,
                            int const interval, int const count) {
#ifdef _WIN32
    /* https://msdn.microsoft.com/en-us/library/windows/desktop/dd877220(v=vs.85).aspx
     */
    struct tcp_keepalive options;
    std::memset (&options, 0, sizeof (options));
    options.onoff         = 1;
    options.keepalivetime = 1000 * idle;

    /* On Windows Vista and later, the number of keep-alive probes (data
     * retransmissions) is set to 10 and cannot be changed. Change the interval
     * instead to maintain the total timeout time.
     */
    options.keepaliveinterval = 100 * interval * count;

    DWORD bytes = 0;
    return (0 == WSAIoctl (socket.native_handle (),
                           SIO_KEEPALIVE_VALS,
                           &options,
                           sizeof (options),
                           NULL,
                           0,
                           &bytes,
                           NULL,
                           NULL));
#elif defined(__APPLE__) || defined(__linux__)
    boost::system::error_code ec;
    bool success = true;

    socket.set_option (boost::asio::ip::tcp::socket::keep_alive (true), ec);
    success &= !ec;

    socket.set_option (tcp_keep_alive_idle (idle), ec);
    success &= !ec;

    socket.set_option (tcp_keep_alive_interval (interval), ec);
    success &= !ec;

    socket.set_option (tcp_keep_alive_count (count), ec);
    success &= !ec;

    return success;
#else
    return false;
#endif
}

template <typename Socket> inline
bool
set_socket_to_close_on_exec (Socket& sock, boost::system::error_code& ec) {
#if defined(__APPLE__) || defined(__linux__)
    errno = 0;
    int flags = ::fcntl(sock.native_handle(), F_GETFD);
    if (flags < 0) {
        ec = boost::system::error_code (errno,
                                        boost::system::generic_category());
        return false;
    }
    flags |= FD_CLOEXEC;
    if (-1 == ::fcntl (sock.native_handle(), F_SETFD, flags)) {
        ec = boost::system::error_code (errno,
                                        boost::system::generic_category());
        return false;
    }
    return true;
#else
    /* Boost Process seems to call CreateProcess with bInheritHandles=FALSE,
     * so we shouldn't need to do anything...
     */
    return true;
#endif
}
