#ifndef LSIF_HPP
#define LSIF_HPP

#include <boost/asio/ip/address.hpp>
#include <map>
#include <string>

std::map<std::string, boost::asio::ip::address>
get_all_netdevices (std::size_t const num_tries = 100);

#endif // LSIF_HPP
