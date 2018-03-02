#pragma once
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <cstdint>
#include <memory>
#include <vector>

class Router;
class ServerProxyConnection;
class ServerProxyMessageHandler;

class ServerProxy final {
public:
    ServerProxy (boost::asio::io_service&, Router& router, uint16_t port);
    ~ServerProxy () noexcept;

    void start (std::int64_t server_id);
    void stop ();

    auto& router () const& noexcept { return router_; }
    auto& connections() const& noexcept { return connections_; }

private:
    uint32_t secondsSinceEpoch();

private:
    boost::asio::ip::tcp::acceptor acceptor_;
    Router& router_;
    std::unique_ptr<ServerProxyMessageHandler> message_handler_;
    std::vector<std::shared_ptr<ServerProxyConnection>> connections_;
    std::int64_t server_id_ = -1;
};
