#pragma once

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/signals2.hpp>
#include <set>

class IPMonitor {
public:
    explicit IPMonitor (boost::asio::io_service& ioService);

    void start();
    void stop();

private:
    void monitorLoop();

    boost::asio::steady_timer m_pollTimer;
    std::set<boost::asio::ip::address> m_knownIPAddresses;
    bool m_running = false;

public:
    boost::signals2::signal<void(std::set<boost::asio::ip::address> const&)>
        ipSetChanged;
};
