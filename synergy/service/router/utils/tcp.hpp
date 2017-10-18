#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>

using tcp = boost::asio::ip::tcp;

template <typename Socket> inline
static void
set_tcp_socket_buffer_sizes (Socket& socket, boost::system::error_code& ec) {
    /* Limit the maximum send and receive buffer size to optimize for low
     * latency LANs.
     *
     * 64 KiB = ~0.5ms at 1 Gb/s,
     *        = ~5ms at 100 Mb/s
     */
    socket.set_option (tcp::socket::receive_buffer_size (64 * 1024), ec);
    socket.set_option (tcp::socket::send_buffer_size (64 * 1024), ec);

    if (ec) {
        std::cout << "Router: Failed to set TCP buffer sizes: " << ec.message ()
                  << "\n";
    }
}
