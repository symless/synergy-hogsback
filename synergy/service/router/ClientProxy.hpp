#pragma once
#include "Asio.hpp"
#include <boost/signals2/signal.hpp>
#include <memory>
#include <vector>
#include <cstdint>

class Router;
struct ClientProxyConnection;
class ClientProxyMessageHandler;

class ClientProxy final {
    friend class ClientProxyMessageHandler;

public:
    explicit ClientProxy (asio::io_service&, Router& router, int port);
    ~ClientProxy();

    void start ();
    void connect (int32_t client_id, const std::string& screen_name);

    Router &router() const;

private:
    asio::io_service& io_;
    int port_;
    std::vector<std::shared_ptr<ClientProxyConnection>> connections_;
    Router& router_;
    std::unique_ptr<ClientProxyMessageHandler> message_handler_;
};
