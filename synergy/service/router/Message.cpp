#include "Message.hpp"


int
Message::type () const noexcept {
    return type_;
}

int
Message::ttl () const noexcept {
    return ttl_;
}
