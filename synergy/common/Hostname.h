#pragma once

#include <boost/asio.hpp>
#include <string>

static std::string
localHostname () {
    return boost::asio::ip::host_name();
}
