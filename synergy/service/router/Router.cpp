#include "Router.hpp"
#include "Connection.hpp"
#include "utils/tcp.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iostream>
#include <iterator>
#include <sstream>
#include <synergy/service/ServiceLogs.h>
#include <tuple>

static int const kConnectRetryLimit     = 10;
static auto const kConnectTimeout       = std::chrono::seconds (5);
static auto const kConnectRetryInterval = std::chrono::seconds (1);
static auto const kHelloInterval        = std::chrono::seconds (2);

static int const kDestRouteLimit = 3;

static std::string
comma_separate (std::vector<uint32_t> const& path) {
    if (path.empty ()) {
        return std::string ();
    }
    std::ostringstream oss;
    auto it = begin (path);
    oss << *it++;
    while (it != end (path)) {
        oss << ",";
        oss << *it++;
    }
    return oss.str ();
}

void
Router::add (std::vector<tcp::endpoint> const& endpoints) {
    for (auto& endpoint : endpoints) {
        add (endpoint);
    }
}

void
Router::add (tcp::endpoint endpoint) {
    auto it = std::find (begin (known_peers_), end (known_peers_), endpoint);
    if (it != end (known_peers_)) {
        return;
    }

    routerLog ()->info ("Adding {}", endpoint);

    known_peers_.push_back (endpoint);

    /* Connect thread */
    asio::spawn (acceptor_.get_io_service (), [this, endpoint](auto ctx) {
        tcp::socket socket (acceptor_.get_io_service ());
        asio::steady_timer timer (socket.get_io_service ());

        socket.open (endpoint.protocol ());
        boost::system::error_code ec;
        set_tcp_socket_buffer_sizes (socket, ec);

        for (int attempt = 1; attempt <= kConnectRetryLimit; ++attempt) {
            if (!running_) {
                return;
            }

            routerLog ()->info ("Connecting to {} (attempt {}/{})",
                                endpoint,
                                attempt,
                                kConnectRetryLimit);

            bool timed_out = false;
            timer.expires_from_now (kConnectTimeout);
            timer.async_wait ([&](auto const ec) {
                if (ec == asio::error::operation_aborted) {
                    return;
                } else if (ec) {
                    throw boost::system::system_error (ec, ec.message ());
                }
                timed_out = true;
                socket.cancel ();
            });

            boost::system::error_code ec;
            socket.async_connect (endpoint, ctx[ec]);
            timer.cancel ();

            if (ec == asio::error::operation_aborted) {
                if (!timed_out) {
                    routerLog ()->info ("Aborting connection to {}", endpoint);

                    return;
                }
            }

            if (ec == asio::error::connection_refused) {
                routerLog ()->info ("Connection to {} refused", endpoint);
            } else if ((ec == asio::error::timed_out) || timed_out) {
                routerLog ()->info ("Connection to {} timed out", endpoint);
            } else if (ec == asio::error::host_unreachable) {
                routerLog ()->info (
                    "Connection to {} failed. Host is unreachable", endpoint);
            } else if (ec) {
                routerLog ()->info (
                    "Connection to {} failed. {} (Code {})", endpoint, ec.message(), ec.value());
            }

            if (ec) {
                timer.expires_from_now (kConnectRetryInterval);
                timer.async_wait (ctx[ec]);
                socket.close ();
                if (ec) {
                    throw boost::system::system_error (ec, ec.message ());
                }
                continue;
            }

            routerLog ()->info ("Successfully connected to {}", endpoint);
            this->add (std::move (socket), false);
            return;
        }

        routerLog ()->info ("Gave up trying to connect to {}", endpoint);

        auto it =
            std::find (begin (known_peers_), end (known_peers_), endpoint);
        if (it != end (known_peers_)) {
            known_peers_.erase (it);
        }
    });
}

Router::Router (asio::io_service& io, int const port)
    : acceptor_ (io), hello_timer_ (io) {

    tcp::endpoint endpoint (ip::address (), port);
    acceptor_.open (endpoint.protocol ());
    acceptor_.set_option (tcp::socket::reuse_address (true));
    acceptor_.bind (endpoint);

    /* On individual connections, the socket buffer size must be set prior to
     * the listen(2) or connect(2) calls in order to have it take effect */
    boost::system::error_code ec;
    set_tcp_socket_buffer_sizes (acceptor_, ec);
    acceptor_.listen ();

    routerLog ()->info ("ID = {}, Name = '{}'", id_, name_);
}

bool
Router::started () const noexcept {
    return running_;
}

void
Router::start (uint32_t const id, std::string name) {
    if (running_) {
        return;
    }

    id_      = id;
    name_    = name;
    running_ = true;

    /* Accept thread */
    asio::spawn (acceptor_.get_io_service (), [this](asio::yield_context ctx) {
        while (running_) {
            tcp::socket socket (acceptor_.get_io_service ());
            boost::system::error_code ec;
            acceptor_.async_accept (socket, ctx[ec]);

            if (!running_ || (ec == asio::error::operation_aborted)) {
                break;
            } else if (ec) {
                throw boost::system::system_error (ec);
            }

            routerLog ()->info ("Accepted connection from router {}",
                                socket.remote_endpoint ());
            add (std::move (socket), true);
        }
    });

    /* Hello thread - Heartbeats neighbours so they know we're still here */
    asio::spawn (acceptor_.get_io_service (), [this](asio::yield_context ctx) {
        HelloMessage hello;
        hello.id   = id_;
        hello.name = name_;

        while (true) {
            auto connections = connections_;
            for (auto& connection : connections) {
                if (!running_) {
                    return;
                }
                connection->send (hello, ctx);
            }

            boost::system::error_code ec;
            hello_timer_.expires_from_now (kHelloInterval);
            hello_timer_.async_wait (ctx[ec]);

            if (!running_ || (ec == boost::asio::error::operation_aborted)) {
                return;
            }

            if (ec) {
                throw boost::system::system_error (ec, ec.message ());
            }
        }

        routerLog ()->info ("Hello loop terminated");
    });
}

void
Router::shutdown () {
    running_ = false;
    hello_timer_.cancel ();
    acceptor_.cancel ();
}


std::vector<copy_ptr<Route>>
Router::get_known_routes () {
    std::vector<copy_ptr<Route>> known_routes;

    for (auto& route : route_table_) {
        known_routes.emplace_back (std::make_unique<Route> (route));
    }

    return known_routes;
}

void
Router::dump_table () {
    if (route_table_.empty ()) {
        routerLog ()->info ("Route table is empty");
        return;
    } else {
        routerLog ()->info ("Route table contents:");
    }

    int route_n = 0;
    for (auto& route : route_table_) {
        ++route_n;

        routerLog ()->info ("   Route {}: dest {}, cost {}, path [{}]",
                            route_n,
                            route.dest,
                            route.cost,
                            comma_separate (route.path));
    }
}

struct MessageHandler {
    explicit MessageHandler (Router* router) : router_ (router) {
    }

    void operator() (HelloMessage&, std::shared_ptr<Connection>) const;
    void operator() (UnknownMessage&, std::shared_ptr<Connection>) const;
    void operator() (RouteAdvertisement&, std::shared_ptr<Connection>) const;
    void operator() (RouteRevocation& rr, std::shared_ptr<Connection>) const;
    template <typename T>
    void
    operator() (T&, std::shared_ptr<Connection>) const {
    }

private:
    Router* router_;
};

void
MessageHandler::
operator() (HelloMessage& hello, std::shared_ptr<Connection> source) const {
    routerLog ()->info (
        "Received hello from '{}' (id: {})", hello.name, hello.id);
    if (hello.id == router_->id ()) {
        routerLog ()->info ("Received own hello message. Closing connection "
                            "{}",
                            source->id ());
        router_->remove (std::move (source));
        return;
    }
}

void
MessageHandler::
operator() (UnknownMessage& message, std::shared_ptr<Connection> source) const {
    routerLog ()->info ("Received an unknown message");
}

void
MessageHandler::
operator() (RouteAdvertisement& ra, std::shared_ptr<Connection> source) const {
    routerLog ()->info ("Received route advertisement from router {}",
                        ra.sender);
    router_->integrate (ra, std::move (source));
}

void
MessageHandler::
operator() (RouteRevocation& rr, std::shared_ptr<Connection> source) const {
    routerLog ()->info (
        "Received {} dead routes from router {}", rr.routes.size (), rr.sender);
    router_->integrate (rr, std::move (source));
}

void
Router::add (tcp::socket socket, bool isServer) {
    auto connection = std::make_shared<Connection> (std::move (socket), isServer);

    connection->on_disconnect.connect (
        [this](std::shared_ptr<Connection> connection) {
            auto remote = connection->endpoint ();
            this->remove (std::move (connection));
            this->add (remote);
        });

    connection->on_message.connect ([this](MessageHeader const& header,
                                           Message& message,
                                           std::shared_ptr<Connection>
                                               connection) {
        if ((header.dest != id ()) && (header.dest != 0xFFFFFFFF)) {
            forward (header, message);
            return;
        }

        MessageHandler handler (this);
        boost::apply_visitor (
            [&handler, connection = std::move (connection) ](auto& body) {
                handler (body, std::move (connection));
            },
            message.body ());

        on_receive (message, header.source);
    });

    connections_.push_back (connection);
    connection->start ();

    asio::spawn (acceptor_.get_io_service (),
                 [this, connection](asio::yield_context ctx) {
                     RouteAdvertisement advert;
                     advert.sender = id_;

                     auto known_routes = get_known_routes ();
                     advert.routes.reserve (1 + known_routes.size ());

                     auto route  = std::make_unique<Route> ();
                     route->dest = id_;
                     route->cost = 0;
                     advert.routes.emplace_back (std::move (route));

                     advert.routes.insert (
                         end (advert.routes),
                         std::make_move_iterator (begin (known_routes)),
                         std::make_move_iterator (end (known_routes)));

                     connection->send (std::move (advert), ctx);
                 });
}

bool
Router::integrate (Route route, std::shared_ptr<Connection> source) {
    auto existing_routes =
        route_table_.get<by_destination> ().equal_range (route.dest);

    size_t rank_lower        = 0;
    size_t rank_higher       = 0;
    bool route_limit_reached = false;

    if (existing_routes.first != existing_routes.second) {
        rank_lower =
            route_table_.get<by_destination> ().rank (existing_routes.first);
        rank_higher =
            route_table_.get<by_destination> ().rank (existing_routes.second);

        route_limit_reached = ((rank_higher - rank_lower) == kDestRouteLimit);
        if (route_limit_reached) {
            routerLog ()->info ("   Route limit reached for dest {}",
                                route.dest);
        }
    }

    RouteTableEntry entry;
    static_cast<Route&> (entry) = std::move (route);
    entry.connection            = source.get ();

    auto installed = route_table_.insert (std::move (entry));

    if (route_limit_reached && installed.second) {
        auto rank_installed =
            route_table_.get<by_destination> ().rank (installed.first);
        if ((rank_installed >= rank_lower) && (rank_installed <= rank_higher)) {
            auto bumped = std::prev (existing_routes.second);

            routerLog ()->info ("   Removed route: dest {}, cost {}, path [{}]",
                                bumped->dest,
                                bumped->cost,
                                comma_separate (bumped->path));

            routerLog ()->info (
                "  ... to make way for: dest {}, cost {}, path [{}]",
                installed.first->dest,
                installed.first->cost,
                comma_separate (installed.first->path));

            route_table_.get<by_destination> ().erase (bumped);
        } else {
            route_table_.get<by_destination> ().erase (installed.first);
        }
    }

    return installed.second;
}

void
Router::integrate (RouteRevocation& rr, std::shared_ptr<Connection> source) {
    if (rr.sender == id_) {
        routerLog ()->info (" Received own route revocation. "
                            "This indicates a loop. Ignoring");
        return;
    }

    routerLog ()->info ("Removing dead routes received from router {}",
                        rr.sender);

    RouteRevocation revocation;
    revocation.sender = id_;
    revocation.routes.reserve (rr.routes.size ());

    bool routes_updated = false;
    auto route_n        = 0;
    for (auto& route : rr.routes) {
        ++route_n;

        routerLog ()->info ("   Route {}: dest {}, cost {}, path [{}]",
                            route_n,
                            route->dest,
                            route->cost,
                            comma_separate (route->path));

        if (route->dest == id_) {
            routerLog ()->info ("   Route {}: ignored because it's to me",
                                route_n);
            continue;
        }

        if (end (route->path) !=
            std::find (begin (route->path), end (route->path), id_)) {
            routerLog ()->info (
                "   Route {}: ignored because it includes a loop", route_n);
            continue;
        }

        auto new_route  = std::make_unique<Route> ();
        new_route->dest = route->dest;
        new_route->cost = route->cost + 1;
        new_route->path.reserve (route->path.size () + 1);
        new_route->path.emplace_back (rr.sender);
        new_route->path.insert (
            end (new_route->path), begin (route->path), end (route->path));

        auto entry = route_table_.get<by_destination> ().find (
            boost::tie (new_route->dest, new_route->cost, new_route->path));

        if (entry == end (route_table_)) {
            routerLog ()->info ("    Route {}: not found", route_n);
        } else {
            routerLog ()->info ("    Route {}: removed", route_n);
            route_table_.get<by_destination> ().erase (entry);
            routes_updated = true;
        }

        revocation.routes.emplace_back (std::move (new_route));
    }

    dump_table ();

    if (routes_updated) {
        flood (std::move (revocation), source->id ());
    }
}

void
Router::integrate (RouteAdvertisement& ra, std::shared_ptr<Connection> source) {
    if (ra.sender == id_) {
        routerLog ()->info (" Received own route advertisement. "
                            "This indicates a loop. Ignoring");
        return;
    }

    routerLog ()->info ("Installing routes received from router {}", ra.sender);

    RouteAdvertisement advert;
    advert.sender = id_;
    advert.routes.reserve (1 + ra.routes.size ());

    bool routes_updated = false;
    auto route_n        = 0;
    for (auto& route : ra.routes) {
        ++route_n;

        routerLog ()->info ("   Route {}: dest {}, cost {}, path [{}]",
                            route_n,
                            route->dest,
                            route->cost,
                            comma_separate (route->path));

        if (route->dest == id_) {
            routerLog ()->info ("   Route {}: ignored because it's to me",
                                route_n);
            continue;
        }

        if (end (route->path) !=
            std::find (begin (route->path), end (route->path), id_)) {
            routerLog ()->info (
                "   Route {}: ignored because it would create a loop", route_n);
            continue;
        }

        auto new_route  = std::make_unique<Route> ();
        new_route->dest = route->dest;
        new_route->cost = route->cost + 1;
        new_route->path.reserve (route->path.size () + 1);
        new_route->path.emplace_back (ra.sender);
        new_route->path.insert (
            end (new_route->path), begin (route->path), end (route->path));

        bool const installed = integrate (*new_route, source);
        if (installed) {
            routerLog ()->info ("    Route {}: installed", route_n);
            advert.routes.emplace_back (std::move (new_route));
        } else {
            routerLog ()->info ("    Route {}: not installed", route_n);

        }

        routes_updated |= installed;
    }

    dump_table ();

    if (routes_updated) {
        flood (std::move (advert), source->id ());
    }
}

void
Router::remove (std::shared_ptr<Connection> connection) {
    routerLog ()->info (" Disabling connection {}", connection->id ());

    connection->stop ();

    auto dead_routes =
        route_table_.get<by_connection_id> ().equal_range (connection->id ());

    auto const n_dead_routes =
        std::distance (dead_routes.first, dead_routes.second);

    routerLog ()->info (
        "Removing {} route(s) broken by removal of connection {}",
        n_dead_routes,
        connection->id ());

    RouteRevocation revocation;
    revocation.sender = id_;
    revocation.routes.reserve (n_dead_routes);

    std::for_each (
        dead_routes.first, dead_routes.second, [&](Route const& route) {
            revocation.routes.emplace_back (std::make_unique<Route> (route));
        });

    route_table_.get<by_connection_id> ().erase (dead_routes.first,
                                                 dead_routes.second);
    dump_table ();

    auto remote = connection->endpoint ();
    connections_.erase (
        std::remove (begin (connections_), end (connections_), connection),
        end (connections_));

    if (!revocation.routes.empty ()) {
        broadcast (std::move (revocation));
    }

    auto it = std::find (begin (known_peers_), end (known_peers_), remote);
    if (it != end (known_peers_)) {
        known_peers_.erase (it);
    }
}

bool
Router::send (Message message, uint32_t dest) {
    MessageHeader header = message.make_header ();
    header.source        = id_;
    header.dest          = dest;
    return forward (header, message);
}

bool
Router::forward (MessageHeader const& header, Message message) {
    auto& by_dest = route_table_.get<by_destination> ();
    auto route    = by_dest.find (header.dest);
    if ((route == end (by_dest)) || !route->connection) {
        return false;
    }

    asio::spawn (acceptor_.get_io_service (), [
        this,
        header,
        message    = std::move (message),
        connection = route->connection->shared_from_this ()
    ](auto ctx) { connection->send (header, std::move (message), ctx); });

    return true;
}

void
Router::flood (Message message, uint32_t const source_port) {
    asio::spawn (acceptor_.get_io_service (),
                 [ this, message = std::move (message), source_port ](
                     asio::yield_context ctx) {
                     auto connections = connections_;
                     for (auto& connection : connections) {
                         if (!running_) {
                             break;
                         }
                         if (connection->id () != source_port) {
                             connection->send (message, ctx);
                         }
                     }
                 });
}

void
Router::broadcast (Message message) {
    asio::spawn (
        acceptor_.get_io_service (),
        [ this, message = std::move (message) ](asio::yield_context ctx) {
            auto connections = connections_;
            for (auto& connection : connections) {
                if (!running_) {
                    break;
                }
                connection->send (message, ctx);
            }
        });
}

uint32_t
connection_id_extractor::
operator() (RouteTableEntry const& rte) const noexcept {
    return rte.connection->id ();
}
