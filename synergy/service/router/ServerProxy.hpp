#pragma once
#include "Asio.hpp"
#include <boost/signals2/signal.hpp>
#include <cstdint>
#include <memory>
#include <vector>

class Router;
class ServerProxyConnection;
class ServerProxyMessageHandler;

class ServerProxy final {
    friend class ServerProxyMessageHandler;

public:
    explicit ServerProxy (asio::io_service&, Router& router, int port);
    ~ServerProxy ();

    Router& router () const;
    void start (int32_t server_id);
    void reset ();

public:
    boost::signals2::signal<int32_t (std::string screen_name,
                                     asio::yield_context ctx)>
        on_client_connected;

private:
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<ServerProxyConnection>> connections_;
    Router& router_;
    std::unique_ptr<ServerProxyMessageHandler> message_handler_;
};
