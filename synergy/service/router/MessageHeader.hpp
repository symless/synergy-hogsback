#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <cstdint>

struct MessageHeader final {
    uint8_t version = 0;
    uint8_t type    = 0;
    uint8_t unused  = 0;
    uint8_t ttl     = 0;
    uint32_t size   = 0;
    uint32_t source = 0xFFFFFFFF;
    uint32_t dest   = 0xFFFFFFFF;
};

static_assert (sizeof (MessageHeader) == 16, "Message header missized");

// clang-format off
BOOST_FUSION_ADAPT_STRUCT (
    MessageHeader,
    (uint8_t, version)
    (uint8_t, type)
    (uint8_t, unused)
    (uint8_t, ttl)
    (uint32_t, size)
    (uint32_t, source)
    (uint32_t, dest)
)
// clang-format on
