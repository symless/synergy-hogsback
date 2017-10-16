#pragma once
#include "Asio.hpp"
#include "MessageHeader.hpp"
#include <boost/mpl/contains.hpp>
#include <boost/variant.hpp>
#include <synergy/service/router/protocol/v1/MessageTypes.hpp>
#include <synergy/service/router/protocol/v2/MessageTypes.hpp>
#include <type_traits>

using namespace synergy::protocol;
using UnknownMessage = std::vector<char>;

class Message final {
    template <typename AsyncReadStream>
    friend class MessageReader;

    template <typename AsyncWriteStream>
    friend class MessageWriter;

public:
    using Body =
        boost::variant<UnknownMessage, v2::HelloMessage,
                       v2::RouteAdvertisement, v2::RouteRevocation, v2::ProxyClientConnect, v2::ProxyServerClaim, v2::CoreMessage>;

public:
    Message () = default;

    template <typename T>
    Message (T&&,
             std::enable_if_t<!std::is_same<std::decay_t<T>, Message>::value,
                              void*> = 0);

    int type () const noexcept;
    int ttl () const noexcept;

    MessageHeader
    make_header () const& {
        MessageHeader header;
        header.ttl = ttl_;
        header.type = type_;
        return header;
    }

    Body const&
    body () const & {
        return body_;
    }

    Body&
    body () & {
        return body_;
    }

private:
    Body body_;
    int type_ = 0;
    int ttl_  = 1;
};

template <typename T>
inline Message::Message (
    T&& body,
    std::enable_if_t<!std::is_same<std::decay_t<T>, Message>::value, void*>)
    : body_ (std::forward<T> (body)), type_ (body_.which ()) {
    static_assert (boost::mpl::contains<typename Body::types,
                                        std::decay_t<T>>::type::value,
                   "Exact content type must be in the Message::Body variant");
}
