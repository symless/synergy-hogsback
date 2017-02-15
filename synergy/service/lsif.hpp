#ifndef LSIF_HPP
#define LSIF_HPP

#include <boost/asio/ip/address.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <string>

using NetworkInterfaceMap =
    boost::bimap<boost::bimaps::multiset_of<std::string>,
                 boost::bimaps::set_of<boost::asio::ip::address>>;

NetworkInterfaceMap get_netdevice_map ();

#endif // LSIF_HPP
