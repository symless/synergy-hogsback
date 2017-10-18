#ifndef SYNERGY_MESSAGE_TYPES_HPP
#define SYNERGY_MESSAGE_TYPES_HPP

#include "MessageImpl.hpp"
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

namespace boost {
namespace fusion {
namespace detail {

template <typename T>
inline std::ostream&
operator<< (std::ostream& os, std::vector<T> const& v) {
    os << "[";
    if (!v.empty ()) {
        auto b = v.begin ();
        os << *b++;
        while (b != v.end ()) {
            os << ", ";
            os << *b++;
        }
    }
    os << "]";
    return os;
}

} // namespace detail
} // namespace fusion
} // namespace boost

#define DECLARE_SYNERGY_MSG(NAME, ...)                                         \
    BOOST_FUSION_DEFINE_STRUCT (                                               \
        (synergy) (protocol) (v1), NAME##MessageArgs, __VA_ARGS__);            \
                                                                               \
    namespace synergy {                                                        \
    namespace protocol {                                                       \
    namespace v1 {                                                             \
                                                                               \
    class NAME##Message final : public MessageImpl {                           \
    public:                                                                    \
        using Args = NAME##MessageArgs;                                        \
        std::unique_ptr<MessageImpl> clone () const override;                  \
                                                                               \
        char const* type_name () const noexcept override;                      \
        char const* type_tag () const noexcept override;                       \
                                                                               \
        std::size_t size () const noexcept override;                           \
        void write_to (char*) const override;                                  \
        void read_from (char const*, char const*, char const*) override;       \
                                                                               \
        Args const&                                                            \
        args () const noexcept {                                               \
            return args_;                                                      \
        }                                                                      \
                                                                               \
        Args&                                                                  \
        args () noexcept {                                                     \
            return args_;                                                      \
        }                                                                      \
                                                                               \
    private:                                                                   \
        static char const* const type_name_;                                   \
        static char const* const type_tag_;                                    \
        Args args_;                                                            \
    };                                                                         \
                                                                               \
    std::ostream& operator<< (std::ostream& os, NAME##Message const&);         \
    }                                                                          \
    }                                                                          \
    }

using u8_type     = std::uint8_t;
using s16_type    = std::int16_t;
using u16_type    = std::uint16_t;
using u32_type    = std::uint32_t;
using string_type = std::string;

#undef major
#undef minor

BOOST_FUSION_DEFINE_STRUCT ((synergy) (protocol) (v1), Version,
                            (u16_type, major) (u16_type, minor))

using synergy::protocol::v1::Version;

DECLARE_SYNERGY_MSG (Hello, (Version, version))

DECLARE_SYNERGY_MSG (HelloBack, (Version, version) (string_type, screen_name))

DECLARE_SYNERGY_MSG (Noop)

DECLARE_SYNERGY_MSG (Close)

DECLARE_SYNERGY_MSG (Enter, (s16_type, x) (s16_type, y) (u32_type, sequence) (
                                u16_type, modifier_mask))

DECLARE_SYNERGY_MSG (Leave)

DECLARE_SYNERGY_MSG (ClipboardGrab,
                     (u8_type, clipboard_id) (u32_type, sequence))

DECLARE_SYNERGY_MSG (Screensaver, (u8_type, on))

DECLARE_SYNERGY_MSG (ResetOptions)

DECLARE_SYNERGY_MSG (InfoAck)

DECLARE_SYNERGY_MSG (KeepAlive)

DECLARE_SYNERGY_MSG (KeyDown,
                     (u16_type, key_id) (u16_type, modifier_mask) (u16_type,
                                                                   button))

DECLARE_SYNERGY_MSG (KeyRepeat, (u16_type, key_id) (u16_type, modifier_mask) (
                                    u16_type, count) (u16_type, button))

DECLARE_SYNERGY_MSG (KeyUp,
                     (u16_type, key_id) (u16_type, modifier_mask) (u16_type,
                                                                   button))

DECLARE_SYNERGY_MSG (MouseDown, (u8_type, button))

DECLARE_SYNERGY_MSG (MouseUp, (u8_type, button))

DECLARE_SYNERGY_MSG (MouseMove, (s16_type, x) (s16_type, y))

DECLARE_SYNERGY_MSG (MouseRelMove, (s16_type, dx) (s16_type, dy))

DECLARE_SYNERGY_MSG (MouseWheelMove, (s16_type, x) (s16_type, y))

DECLARE_SYNERGY_MSG (ClipboardData, (u8_type, clipboard) (u32_type, sequence) (
                                        u8_type, mark) (string_type, data))

DECLARE_SYNERGY_MSG (Info,
                     (s16_type, x) (s16_type, y) (u16_type, width) (
                         u16_type, height) (u16_type, zero) (s16_type,
                                                             mx) (s16_type, my))

DECLARE_SYNERGY_MSG (SetOptions, (std::vector<u32_type>, options))

DECLARE_SYNERGY_MSG (FileTransfer, (u8_type, mark) (string_type, content))

DECLARE_SYNERGY_MSG (DragInfo, (u16_type, file) (string_type, content))

DECLARE_SYNERGY_MSG (InfoQuery)

DECLARE_SYNERGY_MSG (Incompatible, (Version, version))

DECLARE_SYNERGY_MSG (Busy)

DECLARE_SYNERGY_MSG (Unknown)

DECLARE_SYNERGY_MSG (Bad)

#endif
