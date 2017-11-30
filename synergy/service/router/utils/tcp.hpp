#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>
#include <synergy/service/ServiceLogs.h>

template <typename Socket> static inline
void
restrict_tcp_socket_buffer_sizes (Socket& socket,
                                  boost::system::error_code& ec) {
    /* Limit the maximum send and receive buffer size to optimize for low
     * latency LANs.
     *
     * 64 KiB = ~0.5ms at 1 Gb/s,
     *        = ~5ms at 100 Mb/s
     */
    using tcp = boost::asio::ip::tcp;
    socket.set_option (tcp::socket::receive_buffer_size (64 * 1024), ec);
    socket.set_option (tcp::socket::send_buffer_size (64 * 1024), ec);

    if (ec) {
        routerLog()->warn("Failed to restrict TCP socket buffer sizes: {}",
                          ec.message ());
    }
}
