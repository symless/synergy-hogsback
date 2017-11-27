#pragma once
#include "Asio.hpp"
#include "Message.hpp"
#include "protocol/v2/MessageTypes.hpp"
#include <boost/container/small_vector.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ranked_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/signals2.hpp>
#include <cassert>
#include <cstdint>
#include <memory>
#include <tuple>
#include <vector>

class Connection;
class MessageHeader;

using namespace synergy::protocol::v2;
namespace ssl = boost::asio::ssl;

struct RouteTableEntry : Route {
    Connection* connection = nullptr;
};

struct connection_id_extractor {
    using result_type = uint32_t;
    result_type operator() (RouteTableEntry const&) const noexcept;
};

namespace bmi = boost::multi_index;
struct by_destination;
struct by_connection_id;

using RouteTable = bmi::multi_index_container<
    RouteTableEntry,
    bmi::indexed_by<
        bmi::ranked_unique<
            bmi::tag<by_destination>,
            bmi::composite_key<
                Route, bmi::member<Route, uint32_t, &RouteTableEntry::dest>,
                bmi::member<Route, uint32_t, &RouteTableEntry::cost>,
                bmi::member<Route, std::vector<uint32_t>,
                            &RouteTableEntry::path>>>,
        bmi::hashed_non_unique<bmi::tag<by_connection_id>,
                               connection_id_extractor>>>;

class Router final {
public:
    friend class MessageHandler;

    Router (asio::io_service& io, int port);

    auto
    id () const noexcept {
        return id_;
    }

    bool started () const noexcept;
    void start (const uint32_t id, std::string name);
    void shutdown ();
    void add (tcp::endpoint endpoint);
    void add (std::vector<tcp::endpoint> const& endpoints);
    void remove (std::shared_ptr<Connection>);

    bool send (Message message, uint32_t dest);
    void flood (Message message, uint32_t source_port);
    void broadcast (Message message);
    bool forward (MessageHeader const& header, Message message);

protected:
    bool add(tcp::socket, bool isServer, asio::yield_context ctx);

    bool integrate (Route, std::shared_ptr<Connection> source);
    void integrate (RouteRevocation&, std::shared_ptr<Connection>);
    void integrate (RouteAdvertisement&, std::shared_ptr<Connection>);

    void dump_table ();
    std::vector<copy_ptr<Route>> get_known_routes ();

private:
    void loadRawCertificate();

    static const std::string sslCertificate();
    static const std::string sslKey();
    static const std::string sslDH();

private:
    tcp::acceptor acceptor_;
    asio::steady_timer hello_timer_;
    std::vector<std::shared_ptr<Connection>> connections_;
    std::vector<tcp::endpoint> known_peers_;
    RouteTable route_table_;
    uint32_t id_ = 0;
    std::string name_;
    bool running_ = false;
    ssl::context context_;

public:
    boost::signals2::signal<void(Message const&, int32_t)> on_receive;
};
