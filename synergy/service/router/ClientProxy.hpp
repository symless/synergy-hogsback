#pragma once
#include "Asio.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class Router;
class ClientProxyConnection;
class ClientProxyMessageHandler;

class ClientProxy final {
    friend class ClientProxyMessageHandler;

public:
    ClientProxy (asio::io_service&, Router& router, int port);
    ~ClientProxy ();

    void start ();
    void connect (int32_t client_id, const std::string& screen_name, uint32_t connection_id);

    Router& router () const noexcept;

private:
    asio::io_service& io_;
    Router& router_;
    std::vector<std::shared_ptr<ClientProxyConnection>> connections_;
    std::unique_ptr<ClientProxyMessageHandler> message_handler_;
    int port_;
};
