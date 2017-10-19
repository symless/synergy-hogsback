#pragma once

#include <synergy/service/router/protocol/v1/MessageTypes.hpp>

#include <boost/signals2/signal.hpp>
#include <cstddef>
#include <fmt/ostream.h>
#include <iostream>
#include <synergy/service/ServiceLogs.h>
#include <synergy/service/router/Router.hpp>

namespace synergy {
namespace protocol {
namespace v1 {

enum class Flow : int {
    STC = 0, // Server to Client
    CTS = 1  // Client to Server
};

class Handler final {
public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    Handler (Router& router, int32_t const server)
        : router_ (router), server_id_ (server) {
    }

    template <typename T>
    bool
    operator() (T& msg) noexcept {
        routerLog ()->debug ("Core message: {}", msg);

        std::vector<unsigned char> buffer;
        int32_t size = msg.size ();
        buffer.resize (size);
        msg.write_to (reinterpret_cast<char*> (buffer.data ()));

        CoreMessage coreMessage;
        coreMessage.data = std::move (buffer);
        return router_.send (coreMessage, server_id_);
    }

    bool
    operator() (HelloBackMessage& msg) noexcept {
        on_hello_back (msg.args ().screen_name);
        return true;
    }

    bool
    operator() (HelloMessage& msg) noexcept {
        on_hello ();
        return true;
    }

private:
    Router& router_;
    int32_t server_id_;

public:
    signal<void(std::string screenname)> on_hello_back;
    signal<void()> on_hello;
};

bool process (Flow flow, Handler& handler, char const* msg, std::size_t size);

} // namespace v1
} // namespace protocol
} // namespace synergy
