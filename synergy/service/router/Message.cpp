#include "Message.hpp"
#include <boost/numeric/conversion/cast.hpp>

MessageHeader
Message::header() const & {
    MessageHeader header;
    header.ttl  = boost::numeric_cast<decltype(header.ttl)>(ttl_);
    header.type = boost::numeric_cast<decltype(header.type)>(type_);
    return header;
}
