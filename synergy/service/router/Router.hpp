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

namespace bmi = boost::multi_index;
using namespace synergy::protocol::v2;

class Connection;
struct MessageHeader;

struct RouteTableEntry : Route {
    Connection* connection = nullptr;
};

struct connection_id_extractor {
    using result_type = uint32_t;
    result_type operator() (RouteTableEntry const&) const noexcept;
};

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
    Router (asio::io_service& io, std::uint16_t port);

    auto
    id () const noexcept {
        return id_;
    }

    asio::io_service& getIoService();

    bool started () const noexcept;
    void start (const uint32_t id, std::string name);
    void shutdown ();

    void add_peer (boost::asio::ip::tcp::endpoint, bool immediate = false);
    void remove (std::shared_ptr<Connection>);

    bool send (Message message, std::uint32_t dest);
    bool forward (MessageHeader const& header, Message message);

    void broadcast (Message const& message);
    void notifyOtherNodes (Message const& message);
    void flood (Message const& message, std::uint32_t source);

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(Message const&, int32_t)> on_receive;

    signal<void(int64_t screen_id)> on_node_reachable;
    signal<void(int64_t screen_id)> on_node_unreachable;

protected:
    void add (std::shared_ptr<Connection>);
    bool integrate (Route, std::shared_ptr<Connection> source);
    void integrate (RouteRevocation&, std::shared_ptr<Connection>);
    void integrate (RouteAdvertisement&, std::shared_ptr<Connection>);

    void dump_table ();
    std::vector<copy_ptr<Route>> get_known_routes ();

private:
    void loadRawCertificate();
    static std::string sslCertificate();
    static std::string sslKey();
    static std::string sslDH();

private:
    tcp::acceptor acceptor_;
    asio::steady_timer hello_timer_;
    std::vector<std::shared_ptr<Connection>> connections_;
    std::vector<tcp::endpoint> known_peers_;
    RouteTable route_table_;
    boost::asio::ssl::context ssl_context_;
    std::string name_;
    uint32_t id_ = 0;
    bool running_ = false;
};
