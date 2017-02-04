#ifndef LSIF_HPP
#define LSIF_HPP

#include <boost/asio/ip/address.hpp>
#include <map>
#include <string>

// TODO: return a set of network interface structs that look more like the
// struct in multicast.hpp

std::map<std::string, boost::asio::ip::address> get_all_netdevices ();

#endif // LSIF_HPP
