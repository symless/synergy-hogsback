#pragma once
#include "Asio.hpp"
#include "MessageHeader.hpp"
#include <boost/mpl/contains.hpp>
#include <boost/variant.hpp>
#include <synergy/service/router/protocol/v1/MessageTypes.hpp>
#include <synergy/service/router/protocol/v2/MessageTypes.hpp>
#include <type_traits>
#include <vector>

using namespace synergy::protocol;
using UnknownMessage = std::vector<char>;

class Message final {
    template <typename AsyncReadStream>
    friend class MessageReader;

    template <typename AsyncWriteStream>
    friend class MessageWriter;

private:
    /* Do NOT reorder this variant! The packet type is deduced from an index
     * in to it. Only add to the end.
     */
    using Body = boost::variant<
        UnknownMessage,
        v2::HelloMessage,
        v2::RouteAdvertisement,
        v2::RouteRevocation,
        v2::ProxyClientConnect,
        v2::ServerClaim,
        v2::CoreMessage,
        v2::ProxyClientDisconnect,
        v2::ProxyServerReset
    >;

public:
    Message () = default;

    template <typename T>
    Message (T&&,
             std::enable_if_t<!std::is_same<std::decay_t<T>, Message>::value,
                              void*> = 0);

    MessageHeader header () const &;

    Body&
    body () & {
        return body_;
    }

    Body const&
    body () const& {
        return body_;
    }

    auto type () const noexcept {
        return type_;
    }

    auto ttl () const noexcept {
        return ttl_;
    }

private:
    Body body_;
    int type_ = 0;  // default: UnknownMessage
    int ttl_  = 1;  // default: Don't relay across multiple hops
};

template <typename T> inline
Message::Message (
    T&& body,
    std::enable_if_t<!std::is_same<std::decay_t<T>, Message>::value, void*>)
    : body_ (std::forward<T> (body)), type_ (body_.which ()) {
    static_assert (boost::mpl::contains<typename Body::types,
                                        std::decay_t<T>>::type::value,
                   "Exact content type must be in the Message::Body variant");
}
