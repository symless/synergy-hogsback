#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>

namespace asio = boost::asio;
namespace ip   = asio::ip;
using tcp      = ip::tcp;
