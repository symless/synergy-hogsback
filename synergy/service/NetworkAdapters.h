#ifndef LSIF_HPP
#define LSIF_HPP

#include <boost/asio/ip/address.hpp>
#include <map>
#include <string>

std::multimap<std::string, boost::asio::ip::address> getAdapterIPAddresses();

#endif // LSIF_HPP
