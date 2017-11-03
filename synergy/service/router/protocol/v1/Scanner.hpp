#pragma once

#include <synergy/service/router/protocol/v1/MessageTypes.hpp>

#include <boost/signals2/signal.hpp>
#include <cstddef>
#include <fmt/ostream.h>
#include <iostream>
#include <synergy/service/ServiceLogs.h>
#include <synergy/service/router/Router.hpp>
#include <algorithm>

namespace synergy {
namespace protocol {
namespace v1 {

enum class Flow : int {
    STC = 0, // Server to Client
    CTS = 1  // Client to Server
};

/* Returns true if, and only if, all of signal handlers return true */
struct all_true final {
    using result_type = bool;
    template <typename InputIterator>
    bool operator() (InputIterator begin, InputIterator end) const noexcept {
        return (std::find (begin, end, false) == end);
    }
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
        routerLog ()->trace ("Core message: {}", msg);

        std::vector<unsigned char> buffer; /* TODO: reuse this buffer */
        int32_t size = msg.size ();
        buffer.resize (size);
        msg.write_to (reinterpret_cast<char*> (buffer.data ()));

        CoreMessage coreMessage;
        coreMessage.data = std::move (buffer);
        return router_.send (std::move(coreMessage), server_id_);
    }

    bool
    operator() (HelloBackMessage& msg) noexcept {
        return on_hello_back (msg.args ().screen_name);
    }

    bool
    operator() (HelloMessage& msg) noexcept {
        return on_hello ();
    }

private:
    Router& router_;
    int32_t server_id_;

public:
    signal<bool(std::string screenname), all_true> on_hello_back;
    signal<bool(), all_true> on_hello;
};

bool process (Flow flow, Handler& handler, unsigned char const* msg,
              std::size_t size);

} // namespace v1
} // namespace protocol
} // namespace synergy
