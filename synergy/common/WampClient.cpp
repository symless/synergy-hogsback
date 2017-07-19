#include "WampClient.h"

#include <autobahn/autobahn.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <string>

WampClient::WampClient(boost::asio::io_service& io) :
    m_executor (io)
{
    m_default_call_options.set_timeout (std::chrono::seconds(10));
}

WampClient::future_t<void>
WampClient::start (std::string const ip, int const port, bool const debug)
{
    auto transport = std::make_shared<autobahn::wamp_tcp_transport>(
            get_io_service (), boost::asio::ip::tcp::endpoint
                (boost::asio::ip::address_v4::from_string(ip), port), debug);
    m_session = std::make_shared<autobahn::wamp_session>(get_io_service(),
                                                         debug);
    transport->attach(std::static_pointer_cast<autobahn::wamp_transport_handler>
                      (m_session));

    return transport->connect().then ([&](boost::future<void> connected) {
        connected.get();
        m_session->start().then ([&](boost::future<void> started) {
            started.get();
            m_session->join("default").then ([&](boost::future<uint64_t> joined) {
                joined.get();
            });
        });
    });
}
