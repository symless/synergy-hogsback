#pragma once

#include <synergy/service/router/protocol/v1/MessageTypes.hpp>

#include <boost/signals2/signal.hpp>
#include <cstddef>
#include <iostream>
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
    Handler (Router& router, int32_t target_id)
        : router_ (router), target_id_ (target_id) {
    }

    template <typename T>
    void
    operator() (T& msg) noexcept {
        std::cout << msg << "\n";

        std::vector<unsigned char> buffer;
        int32_t size = msg.size ();
        buffer.resize (size);
        msg.write_to (reinterpret_cast<char*> (buffer.data ()));

        CoreMessage coreMessage;
        coreMessage.data = buffer;
        router_.send (coreMessage, target_id_);
    }

    void
    operator() (HelloBackMessage& msg) noexcept {
        on_hello_back (msg.args ().screen_name);
    }

    void
    operator() (HelloMessage& msg) noexcept {
        on_hello ();
    }

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(std::string screenname)> on_hello_back;
    signal<void()> on_hello;

private:
    Router& router_;
    int32_t target_id_;
};


// TODO: figure out how to use parse() with both ClientProxy && ServerProxy
//       e.g. use type erasure or variant message type.
bool parse (Flow flow, Handler& handler, char const* msg, std::size_t size);

} // namespace v1
} // namespace protocol
} // namespace synergy
