#pragma once
#include "Asio.hpp"
#include <boost/signals2/signal.hpp>
#include <cstdint>
#include <memory>
#include <vector>

class Router;
struct ServerProxyConnection;
struct ServerProxyMessageHandler;

class ServerProxy final {
    friend class ServerProxyMessageHandler;

public:
    explicit ServerProxy (asio::io_service&, Router& router, int port);
    ~ServerProxy ();

    void start ();

    Router& router () const;
    uint32_t server_id () const;

public:
    boost::signals2::signal<int32_t (std::string screen_name,
                                     asio::yield_context ctx)>
        on_client_connected;
    boost::signals2::signal<void(uint32_t)> on_new_server;

private:
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<ServerProxyConnection>> connections_;
    Router& router_;
    uint32_t server_id_;
    std::unique_ptr<ServerProxyMessageHandler> message_handler_;
};
