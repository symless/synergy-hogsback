#include "Router.hpp"
#include "Connection.hpp"
#include "utils/tcp.hpp"
#include <boost/scope_exit.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iostream>
#include <iterator>
#include <sstream>
#include <synergy/common/SocketOptions.hpp>
#include <synergy/service/ServiceLogs.h>
#include <tuple>

static auto const kHelloInterval        = std::chrono::seconds (3);
static auto const kConnectTimeout       = std::chrono::seconds (5);
static auto const kConnectRetryInterval = std::chrono::seconds (1);
static int const kConnectRetryLimit     = 10;
static int const kDestRouteLimit        = 3;

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
Router::add_peer (tcp::endpoint endpoint, bool const immediate) {
    auto existing = connections_.equal_range
                    (boost::make_tuple (endpoint.address()));
    if (existing.first != existing.second) {
        routerLog()->warn("Ignoring request to connect to {} (already connected)",
                          endpoint.address());
        return;
    }

    routerLog()->debug("Connecting to new peer at {}", endpoint);

    /* Connect thread */
    asio::spawn (acceptor_.get_io_service (), [this, endpoint, immediate](auto ctx) {
        if (!running_) {
            routerLog()->debug("Aborting connection to {} (router shutdown)",
                               endpoint);
            return;
        }

        asio::steady_timer timer (acceptor_.get_io_service ());

        /* Delay the first connection attempt if requested. This happens e.g.
         * when reconnecting after an unexpected disconnect.
         */
        if (!immediate) {
            boost::system::error_code ec;
            timer.expires_from_now (kConnectRetryInterval);
            timer.async_wait (ctx[ec]);
        }

        int attempt = 1;

        for (; attempt <= kConnectRetryLimit; ++attempt) {
            if (!running_) {
                routerLog()->debug("Aborting connection to {} (router shutdown)",
                                   endpoint);
                break;
            }

            tcp::socket socket (acceptor_.get_io_service ());
            socket.open (endpoint.protocol ());
            boost::system::error_code ec;
            restrict_tcp_socket_buffer_sizes (socket, ec);

            routerLog()->debug("Connecting to {} (attempt {}/{})",
                                endpoint, attempt, kConnectRetryLimit);

            timer.expires_from_now (kConnectTimeout);
            timer.async_wait ([&socket](auto const ec) {
                if (ec == asio::error::operation_aborted) {
                    return;
                } else if (ec) {
                    throw boost::system::system_error (ec, ec.message ());
                }
                socket.cancel ();
            });

            socket.async_connect (endpoint, ctx[ec]);
            timer.cancel ();

            if (ec == asio::error::operation_aborted) {
                routerLog()->info("Connection to {} timed out.", endpoint);
                continue;
            }

            if (ec) {
                routerLog()->error("Connection to {} failed: {} (code {})",
                                   endpoint, ec.message(), ec.value());
                socket.shutdown (boost::asio::ip::tcp::socket::shutdown_both, ec);
                socket.close (ec);
                timer.expires_from_now (kConnectRetryInterval);
                timer.async_wait (ctx[ec]);
                continue;
            }

            auto existing = connections_.equal_range
                (boost::make_tuple (socket.remote_endpoint().address(),
                                    socket.local_endpoint().address()));

            if (existing.first != existing.second) {
                routerLog()->warn ("Connected and dropped duplicate connection"
                                    " to {}",
                                    socket.remote_endpoint ().address());
                socket.shutdown (boost::asio::ip::tcp::socket::shutdown_both,
                                 ec);
                socket.close ();
                break;
            }

            routerLog ()->debug ("Connected to {}", socket.remote_endpoint());

            auto connection = std::make_shared<Connection> (std::move (socket),
                                                            ssl_context_);
            connection->stream().async_handshake
                (Connection::stream_type::client, ctx[ec]);

            if (ec) {
                routerLog ()->error ("Connection {} failed to complete SSL "
                                     "handshake: {}",  connection->id (),
                                     ec.message());
                timer.expires_from_now (kConnectRetryInterval);
                timer.async_wait (ctx[ec]);
                continue;
            }

            this->add (std::move(connection));
            break;
        }

        if (kConnectRetryLimit < attempt) {
            routerLog()->debug("Gave up trying to connect to {}", endpoint);
        }
    });
}

Router::Router (asio::io_service& io, std::uint16_t const port)
    : acceptor_(io), hello_timer_(io),
      ssl_context_(boost::asio::ssl::context::tls)
{
    try {
        loadRawCertificate();
        tcp::endpoint endpoint (ip::address(), port);
        acceptor_.open (endpoint.protocol ());
        acceptor_.set_option (tcp::socket::reuse_address (true));

        boost::system::error_code ec;
        if (!set_socket_to_close_on_exec (acceptor_, ec)) {
            routerLog ()->critical ("Failed to set router port to "
                                    "close-on-exec: {}", ec.message());
        }

        routerLog()->debug("binding to {}:{}", endpoint.address().to_string(),
                           endpoint.port());
        acceptor_.bind (endpoint);

        /* The socket buffer size must be set prior to the listen() or connect()
         * calls in order to have it take effect
         */
        restrict_tcp_socket_buffer_sizes (acceptor_, ec);
        acceptor_.listen ();

        routerLog()->debug("Initialized");
    } catch (std::exception& e) {
        routerLog()->error("Failed to initialize: {}", e.what());
        throw;
    } catch (...) {
        routerLog()->error("Failed to initialize");
        throw;
    }
}

asio::io_service&
Router::getIoService()
{
    return acceptor_.get_io_service();
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

    name_    = std::move(name);
    id_      = id;
    running_ = true;

    routerLog()->debug ("Starting...");
    routerLog()->debug ("ID = {}, Name = '{}'", id_, name_);

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

            auto existing = connections_.equal_range
                (boost::make_tuple (socket.remote_endpoint().address(),
                                    socket.local_endpoint().address()));

            if (existing.first != existing.second) {
                routerLog()->warn ("Accepted and dropped duplicate connection "
                                    "from {}",
                                    socket.remote_endpoint ().address());
                socket.shutdown (boost::asio::ip::tcp::socket::shutdown_both, ec);
                socket.close();
                continue;
            }

            routerLog()->debug("Accepted connection from router {}",
                                socket.remote_endpoint ());

            auto connection = std::make_shared<Connection> (std::move(socket),
                                                            ssl_context_);
            connection->stream().async_handshake (Connection::stream_type::server,
                                                  ctx[ec]);
            if (ec) {
                routerLog ()->error ("Connection {} failed to complete SSL "
                                     "handshake: {}",  connection->id (),
                                     ec.message());
                continue;
            }

            add (std::move (connection));
        }

        routerLog()->debug("Accept loop terminated");
    });

    /* Hello thread - Heartbeats neighbours so they know we're still here */
    asio::spawn (acceptor_.get_io_service (), [this](asio::yield_context ctx) {
        HelloMessage hello;
        hello.id   = id_;
        hello.name = name_;

        while (true) {
            auto connections = connections_; // FIXME
            for (auto& connection : connections) {
                if (!running_) {
                    return;
                }
                connection->send (hello);
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

        routerLog()->debug("Hello loop terminated");
    });
}

void
Router::shutdown () {
    running_ = false;
    hello_timer_.cancel ();
    acceptor_.cancel ();
    acceptor_.get_io_service().poll();
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
        routerLog()->debug("Route table is empty");
        return;
    } else {
        routerLog()->debug("Route table contents:");
    }

    int route_n = 0;
    for (auto& route : route_table_) {
        ++route_n;
        routerLog()->debug("   Route {}: dest {}, cost {}, path [{}]",
                            route_n,
                            route.dest,
                            route.cost,
                            comma_separate (route.path));
    }
}

struct MessageHandler {
    explicit MessageHandler (Router* const router) : router_ (router) {
    }

    template <typename T>
    void
    operator() (T&, std::shared_ptr<Connection>) const {
    }

    void operator() (HelloMessage&, std::shared_ptr<Connection>) const;
    void operator() (UnknownMessage&, std::shared_ptr<Connection>) const;
    void operator() (RouteAdvertisement&, std::shared_ptr<Connection>) const;
    void operator() (RouteRevocation& rr, std::shared_ptr<Connection>) const;

private:
    Router* router_;
};

void
MessageHandler::
operator() (HelloMessage& hello, std::shared_ptr<Connection> source) const {
    routerLog()->trace(
        "Received hello from '{}' (id: {})", hello.name, hello.id);
    if (hello.id == router_->id ()) {
        routerLog()->debug("Received own hello message. Closing connection "
                            "{}",
                            source->id ());
        router_->remove (std::move (source));
        return;
    }
}

void
MessageHandler::
operator() (UnknownMessage& message, std::shared_ptr<Connection> source) const {
    routerLog()->debug("Received an unknown message");
}

void
MessageHandler::
operator() (RouteAdvertisement& ra, std::shared_ptr<Connection> source) const {
    routerLog()->debug("Received route advertisement from router {}",
                        ra.sender);
    router_->integrate (ra, std::move (source));
}

void
MessageHandler::
operator() (RouteRevocation& rr, std::shared_ptr<Connection> source) const {
    routerLog()->debug(
        "Received {} dead routes from router {}", rr.routes.size (), rr.sender);
    router_->integrate (rr, std::move (source));
}

void
Router::add
(std::shared_ptr<Connection> connection) {
    connections_.insert (connection);

    connection->on_connected.connect (
        [this](std::shared_ptr<Connection> connection) {
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
                std::make_move_iterator (end (known_routes))
            );

            connection->send (std::move (advert));
        });

    connection->on_disconnect.connect (
        [this](std::shared_ptr<Connection> connection) {
            routerLog ()->debug ("Connection {} disconnected",
                                 connection->id());
            auto remote = connection->remote_acceptor_endpoint ();
            this->remove (std::move (connection));
            this->add_peer (std::move (remote));
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

    return connection->start ();
}

bool
Router::integrate (Route route, std::shared_ptr<Connection> source) {
    size_t rank_lower        = 0;
    size_t rank_higher       = 0;
    bool route_limit_reached = false;
    bool new_destination     = false;
    bool installed           = false;

    auto existing_routes =
        route_table_.get<by_destination> ().equal_range (route.dest);

    if (existing_routes.first == existing_routes.second) {
        new_destination = true;
    } else {
        rank_lower =
            route_table_.get<by_destination> ().rank (existing_routes.first);
        rank_higher =
            route_table_.get<by_destination> ().rank (existing_routes.second);

        route_limit_reached = ((rank_higher - rank_lower) == kDestRouteLimit);
        if (route_limit_reached) {
            routerLog()->debug ("   Route limit reached for dest {}",
                                route.dest);
        }
    }

    RouteTableEntry entry;
    static_cast<Route&> (entry) = std::move (route);
    entry.connection            = source.get ();

    auto installed_pair = route_table_.insert (entry);
    installed = installed_pair.second;

    if (route_limit_reached && installed) {
        auto rank_installed =
            route_table_.get<by_destination> ().rank (installed_pair.first);
        assert (rank_installed >= rank_lower);
        assert (rank_installed <= rank_higher);

        if (rank_installed < rank_higher) {
            auto bumped = std::prev (existing_routes.second);
            routerLog()->debug("   Removed route: dest {}, cost {}, path [{}]",
                                bumped->dest,
                                bumped->cost,
                                comma_separate (bumped->path));

            routerLog()->debug(
                "  ... to make way for: dest {}, cost {}, path [{}]",
                installed_pair.first->dest,
                installed_pair.first->cost,
                comma_separate (installed_pair.first->path)
            );

            route_table_.get<by_destination> ().erase (bumped);
        } else {
            route_table_.get<by_destination> ().erase (installed_pair.first);
            installed = false;
        }
    }

    new_destination &= installed;
    if (new_destination) {
        routerLog()->debug ("Node {} is now reachable", entry.dest);
        on_node_reachable (entry.dest);
    }

    return installed;
}

void
Router::integrate (RouteRevocation& rr, std::shared_ptr<Connection> source) {
    if (rr.sender == id_) {
        routerLog()->debug("Received own route revocation. "
                            "This indicates a routing loop. Ignoring");
        return;
    }

    routerLog()->debug("Removing dead routes received from node {}",
                        rr.sender);

    RouteRevocation revocation;
    revocation.sender = id_;
    revocation.routes.reserve (rr.routes.size ());

    /* Keep track of the destinations effected by the removal of this
     * connection */
    std::vector<uint32_t> dests_effected;
    dests_effected.reserve (rr.routes.size ());

    bool routes_updated = false;
    auto route_n        = 0;
    for (auto& route : rr.routes) {
        ++route_n;

        routerLog()->debug("   Route {}: dest {}, cost {}, path [{}]",
                            route_n,
                            route->dest,
                            route->cost,
                            comma_separate (route->path));

        if (route->dest == id_) {
            routerLog()->debug("   Route {}: not installed (route loop)",
                                route_n);
            continue;
        }

        if (end (route->path) !=
            std::find (begin (route->path), end (route->path), id_)) {
            routerLog()->debug(
                "   Route {}: ignored because it indicates a routing loop", route_n);
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
            routerLog()->debug("    Route {}: not found", route_n);
        } else {
            dests_effected.push_back (entry->dest);
            route_table_.get<by_destination> ().erase (entry);
            routerLog()->debug ("    Route {}: removed", route_n);
            routes_updated = true;
        }

        /* TODO: Should this be inside the erase() branch, above? Do we need to
         * propagate routes that we couldn't remove, alongside those that we
         * could? For now it seems safer to propagate them */
        revocation.routes.emplace_back (std::move (new_route));
    }

    dump_table ();

    if (routes_updated) {
        routerLog()->debug ("Forwarding route revocation");
        flood (std::move (revocation), source->id ());
    }

    /* Turn the effected destinations in to a set */
    std::sort (begin(dests_effected), end(dests_effected));
    dests_effected.erase (std::unique (begin(dests_effected),
                                       end(dests_effected)),
                          end(dests_effected));

    /* Remove effected destinations if there is still a way to reach them */
    dests_effected.erase (std::remove_if (begin(dests_effected),
                                          end(dests_effected),
        [this](auto dest) {
            return route_table_.get<by_destination>().count(dest);
        }),
        end (dests_effected)
    );

    /* What remains is now a set of now unreachable nodes. Log it. */
    for (auto dest: dests_effected) {
        routerLog()->debug ("Node {} is now unreachable (announced by"
                            " node {})", dest, rr.sender);
        on_node_unreachable (dest);
    }
}

void
Router::integrate (RouteAdvertisement& ra, std::shared_ptr<Connection> source) {
    if (ra.sender == id_) {
        routerLog()->debug("Received own route advertisement. "
                           "This indicates a routing loop. Ignoring");
        return;
    }

    routerLog()->debug("Installing routes received from node {}", ra.sender);
    RouteAdvertisement advert;
    advert.sender = id_;
    advert.routes.reserve (1 + ra.routes.size ());

    bool routes_updated = false;
    auto route_n        = 0;
    for (auto& route : ra.routes) {
        ++route_n;

        routerLog()->debug("   Route {}: dest {}, cost {}, path [{}]",
                            route_n,
                            route->dest,
                            route->cost,
                            comma_separate (route->path));

        if (route->dest == id_) {
            routerLog()->debug("   Route {}: ignored because it leads back to "
                               "to me", route_n);
            continue;
        }

        if (end (route->path) !=
            std::find (begin (route->path), end (route->path), id_)) {
            routerLog()->debug(
                "   Route {}: ignored because it would create a routing loop",
                        route_n);
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
            routerLog()->debug("   Route {}: installed", route_n);
            advert.routes.emplace_back (std::move (new_route));
        } else {
            routerLog()->debug("   Route {}: not installed", route_n);
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
    routerLog()->debug("Removing connection {}", connection->id ());
    connection->stop ();

    auto dead_routes =
        route_table_.get<by_connection_id> ().equal_range (connection->id ());

    auto const n_dead_routes =
        std::distance (dead_routes.first, dead_routes.second);

    routerLog()->debug(
        "Removing {} route(s) broken by removal of connection {}",
        n_dead_routes,
        connection->id ()
    );

    RouteRevocation revocation;
    revocation.sender = id_;
    revocation.routes.reserve (n_dead_routes);

    /* Keep track of the destinations effected by the removal of this connection
     */
    std::vector<uint32_t> dests_effected;
    dests_effected.reserve (n_dead_routes);

    std::for_each (
        dead_routes.first, dead_routes.second, [&](Route const& route) {
            dests_effected.push_back (route.dest);
            revocation.routes.emplace_back (std::make_unique<Route> (route));
        }
    );

    route_table_.get<by_connection_id> ().erase (dead_routes.first,
                                                 dead_routes.second);
    dump_table ();

    auto it = std::find (begin(connections_), end(connections_), connection);
    if (it != end(connections_)) {
        connections_.erase (it);
    }
    connection.reset();

    if (!revocation.routes.empty ()) {
        broadcast (std::move (revocation));
    }

    /* Turn the effected destinations in to a set */
    std::sort (begin(dests_effected), end(dests_effected));
    dests_effected.erase (std::unique (begin(dests_effected),
                                       end(dests_effected)),
                          end(dests_effected));

    /* Remove effected destinations if there is still a way to reach them */
    dests_effected.erase (std::remove_if (begin(dests_effected),
                                          end(dests_effected),
        [this](auto dest) {
            return route_table_.get<by_destination>().count(dest);
        }),
        end (dests_effected)
    );

    /* What remains is now a set of now unreachable nodes. Log it. */
    for (auto dest: dests_effected) {
        routerLog()->debug ("Node {} is now unreachable", dest);
        on_node_unreachable (dest);
    }
}

bool
Router::send (Message message, std::uint32_t const dest) {
    MessageHeader header = message.header ();
    header.source        = this->id_;
    header.dest          = dest;
    return forward (header, message);
}

bool
Router::forward (MessageHeader const& header, Message message) {
    auto& by_dest = route_table_.get<by_destination> ();
    auto route    = by_dest.find (header.dest);

    if ((route == end (by_dest)) || !route->connection) {
        routerLog()->error ("Couldn't send message to node {}: no route to node\n",
                            header.dest);
        dump_table();
        return false;
    }

    /* TODO: enforce TTL */
    route->connection->send (header, std::move (message));
    return true;
}

void Router::notifyOtherNodes(Message message)
{
    uint32_t lastDestId = -1;
    for (auto& route : route_table_) {
        if (lastDestId != route.dest) {
            lastDestId = route.dest;
            send(message, lastDestId);
        }
    }
}

void
Router::flood (Message message, std::uint32_t const source) {
    auto connections = connections_; // FIXME
    for (auto& connection : connections) {
        if (!running_) {
            break;
        }
        if (connection->id () != source) {
            connection->send (message);
        }
    }
}

void
Router::broadcast (Message message) {
    auto connections = connections_; // FIXME
    for (auto& connection : connections) {
        if (!running_) {
            break;
        }
        connection->send (message);
    }
}

uint32_t
connection_id_extractor::operator()
(RouteTableEntry const& rte) const noexcept {
    return rte.connection->id ();
}

void
Router::loadRawCertificate()
{
    const std::string cert = sslCertificate();
    const std::string key = sslKey();
    const std::string dh = sslDH();

    ssl_context_.set_options (
        boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv3 |
        boost::asio::ssl::context::single_dh_use
    );

    ssl_context_.use_certificate_chain (boost::asio::buffer(cert.data(),
                                                            cert.size()));

    ssl_context_.use_private_key (boost::asio::buffer(key.data(), key.size()),
                                  boost::asio::ssl::context::file_format::pem);

    ssl_context_.use_tmp_dh (boost::asio::buffer(dh.data(), dh.size()));

    ssl_context_.set_password_callback (
        [](std::size_t, boost::asio::ssl::context_base::password_purpose) {
                return "test";
        }
    );
}

std::string
Router::sslCertificate()
{
    return "-----BEGIN CERTIFICATE-----\n"
        "MIIDaDCCAlCgAwIBAgIJAO8vBu8i8exWMA0GCSqGSIb3DQEBCwUAMEkxCzAJBgNV\n"
        "BAYTAlVTMQswCQYDVQQIDAJDQTEtMCsGA1UEBwwkTG9zIEFuZ2VsZXNPPUJlYXN0\n"
        "Q049d3d3LmV4YW1wbGUuY29tMB4XDTE3MDUwMzE4MzkxMloXDTQ0MDkxODE4Mzkx\n"
        "MlowSTELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMS0wKwYDVQQHDCRMb3MgQW5n\n"
        "ZWxlc089QmVhc3RDTj13d3cuZXhhbXBsZS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\n"
        "A4IBDwAwggEKAoIBAQDJ7BRKFO8fqmsEXw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcF\n"
        "xqGitbnLIrOgiJpRAPLy5MNcAXE1strVGfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7b\n"
        "Fu8TsCzO6XrxpnVtWk506YZ7ToTa5UjHfBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO\n"
        "9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wWKIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBp\n"
        "yY8anC8u4LPbmgW0/U31PH0rRVfGcBbZsAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrv\n"
        "enu2tOK9Qx6GEzXh3sekZkxcgh+NlIxCNxu//Dk9AgMBAAGjUzBRMB0GA1UdDgQW\n"
        "BBTZh0N9Ne1OD7GBGJYz4PNESHuXezAfBgNVHSMEGDAWgBTZh0N9Ne1OD7GBGJYz\n"
        "4PNESHuXezAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCmTJVT\n"
        "LH5Cru1vXtzb3N9dyolcVH82xFVwPewArchgq+CEkajOU9bnzCqvhM4CryBb4cUs\n"
        "gqXWp85hAh55uBOqXb2yyESEleMCJEiVTwm/m26FdONvEGptsiCmF5Gxi0YRtn8N\n"
        "V+KhrQaAyLrLdPYI7TrwAOisq2I1cD0mt+xgwuv/654Rl3IhOMx+fKWKJ9qLAiaE\n"
        "fQyshjlPP9mYVxWOxqctUdQ8UnsUKKGEUcVrA08i1OAnVKlPFjKBvk+r7jpsTPcr\n"
        "9pWXTO9JrYMML7d+XRSZA1n3856OqZDX4403+9FnXCvfcLZLLKTBvwwFgEFGpzjK\n"
        "UEVbkhd5qstF6qWK\n"
        "-----END CERTIFICATE-----\n";
}

std::string
Router::sslKey()
{
    return "-----BEGIN PRIVATE KEY-----\n"
        "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDJ7BRKFO8fqmsE\n"
        "Xw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcFxqGitbnLIrOgiJpRAPLy5MNcAXE1strV\n"
        "GfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7bFu8TsCzO6XrxpnVtWk506YZ7ToTa5UjH\n"
        "fBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wW\n"
        "KIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBpyY8anC8u4LPbmgW0/U31PH0rRVfGcBbZ\n"
        "sAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrvenu2tOK9Qx6GEzXh3sekZkxcgh+NlIxC\n"
        "Nxu//Dk9AgMBAAECggEBAK1gV8uETg4SdfE67f9v/5uyK0DYQH1ro4C7hNiUycTB\n"
        "oiYDd6YOA4m4MiQVJuuGtRR5+IR3eI1zFRMFSJs4UqYChNwqQGys7CVsKpplQOW+\n"
        "1BCqkH2HN/Ix5662Dv3mHJemLCKUON77IJKoq0/xuZ04mc9csykox6grFWB3pjXY\n"
        "OEn9U8pt5KNldWfpfAZ7xu9WfyvthGXlhfwKEetOuHfAQv7FF6s25UIEU6Hmnwp9\n"
        "VmYp2twfMGdztz/gfFjKOGxf92RG+FMSkyAPq/vhyB7oQWxa+vdBn6BSdsfn27Qs\n"
        "bTvXrGe4FYcbuw4WkAKTljZX7TUegkXiwFoSps0jegECgYEA7o5AcRTZVUmmSs8W\n"
        "PUHn89UEuDAMFVk7grG1bg8exLQSpugCykcqXt1WNrqB7x6nB+dbVANWNhSmhgCg\n"
        "VrV941vbx8ketqZ9YInSbGPWIU/tss3r8Yx2Ct3mQpvpGC6iGHzEc/NHJP8Efvh/\n"
        "CcUWmLjLGJYYeP5oNu5cncC3fXUCgYEA2LANATm0A6sFVGe3sSLO9un1brA4zlZE\n"
        "Hjd3KOZnMPt73B426qUOcw5B2wIS8GJsUES0P94pKg83oyzmoUV9vJpJLjHA4qmL\n"
        "CDAd6CjAmE5ea4dFdZwDDS8F9FntJMdPQJA9vq+JaeS+k7ds3+7oiNe+RUIHR1Sz\n"
        "VEAKh3Xw66kCgYB7KO/2Mchesu5qku2tZJhHF4QfP5cNcos511uO3bmJ3ln+16uR\n"
        "GRqz7Vu0V6f7dvzPJM/O2QYqV5D9f9dHzN2YgvU9+QSlUeFK9PyxPv3vJt/WP1//\n"
        "zf+nbpaRbwLxnCnNsKSQJFpnrE166/pSZfFbmZQpNlyeIuJU8czZGQTifQKBgHXe\n"
        "/pQGEZhVNab+bHwdFTxXdDzr+1qyrodJYLaM7uFES9InVXQ6qSuJO+WosSi2QXlA\n"
        "hlSfwwCwGnHXAPYFWSp5Owm34tbpp0mi8wHQ+UNgjhgsE2qwnTBUvgZ3zHpPORtD\n"
        "23KZBkTmO40bIEyIJ1IZGdWO32q79nkEBTY+v/lRAoGBAI1rbouFYPBrTYQ9kcjt\n"
        "1yfu4JF5MvO9JrHQ9tOwkqDmNCWx9xWXbgydsn/eFtuUMULWsG3lNjfst/Esb8ch\n"
        "k5cZd6pdJZa4/vhEwrYYSuEjMCnRb0lUsm7TsHxQrUd6Fi/mUuFU/haC0o0chLq7\n"
        "pVOUFq5mW8p0zbtfHbjkgxyF\n"
        "-----END PRIVATE KEY-----\n";
}

std::string
Router::sslDH()
{
    return "-----BEGIN DH PARAMETERS-----\n"
        "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
        "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
        "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
        "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
        "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
        "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
        "-----END DH PARAMETERS-----\n";
}

connection_remote_ip_extractor::result_type
connection_remote_ip_extractor::operator()
(std::shared_ptr<Connection> const& connection) const {
    return connection->remote_ip();
}

connection_local_ip_extractor::result_type
connection_local_ip_extractor::operator()
(std::shared_ptr<Connection> const& connection) const {
    return connection->local_ip();
}
