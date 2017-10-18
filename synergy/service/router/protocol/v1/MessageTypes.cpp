#include "MessageTypes.hpp"
#include "MessageCodec.hpp"
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/fusion/algorithm/transformation/zip.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <boost/fusion/sequence/io.hpp>
#include <boost/mpl/range_c.hpp>
#include <cassert>
#include <iomanip>
#include <stdexcept>

#ifdef _MSC_VER
#define UNLIKELY(X) X
#else
#define UNLIKELY(X) __builtin_expect (!!(X), 0)
#endif

template <class Struct>
struct zip_names_and_values_helper {
    template <typename I>
    auto
    operator() (I) const noexcept {
        using boost::fusion::extension::struct_member_name;
        return struct_member_name<Struct, I::value>::call ();
    }
};

template <class Struct>
static inline auto
zip_names_and_values (Struct const& s) {
    using boost::mpl::range_c;
    using namespace boost::fusion;
    using range = range_c<int, 0, result_of::size<Struct>::type::value>;
    static auto names =
        transform (range (), zip_names_and_values_helper<Struct> ());
    return zip (names, s);
}

#define DEFINE_SYNERGY_MSG(NAME, TAG)                                          \
    char const* NAME##Message::type_name () const noexcept {                   \
        return type_name_;                                                     \
    }                                                                          \
                                                                               \
                                                                               \
    char const* NAME##Message::type_tag () const noexcept {                    \
        return type_tag_;                                                      \
    }                                                                          \
                                                                               \
                                                                               \
    std::size_t NAME##Message::size () const noexcept {                        \
        std::size_t n = std::strlen (type_tag ());                             \
                                                                               \
        boost::fusion::for_each (                                              \
            args (), [&n](auto const& arg) { n += codec::size_of (arg); });    \
                                                                               \
        return n;                                                              \
    }                                                                          \
                                                                               \
                                                                               \
    void NAME##Message::read_from (                                            \
        char const* const header, char const* data, char const* const end) {   \
        std::size_t const tag_size = std::strlen (type_tag ());                \
        assert (data > header);                                                \
        assert (end >= data);                                                  \
        std::size_t size = end - data;                                         \
                                                                               \
        if (UNLIKELY (((data - header) != tag_size) ||                         \
                      (std::memcmp (header, type_tag (), tag_size)))) {        \
            throw std::runtime_error ("Bad message header");                   \
        }                                                                      \
                                                                               \
        boost::fusion::for_each (args (), [&data, &size](auto& arg) {          \
            codec::read_from (data, size, arg);                                \
        });                                                                    \
    }                                                                          \
                                                                               \
                                                                               \
    void NAME##Message::write_to (char* buf) const {                           \
        std::size_t const tag_size = std::strlen (type_tag ());                \
        std::memcpy (buf, type_tag (), tag_size);                              \
        buf += tag_size;                                                       \
                                                                               \
        boost::fusion::for_each (                                              \
            args (), [&buf](auto const& arg) { codec::write_to (buf, arg); }); \
    }                                                                          \
                                                                               \
                                                                               \
    std::unique_ptr<MessageImpl> NAME##Message::clone () const {               \
        return std::unique_ptr<MessageImpl> (new NAME##Message (*this));       \
    }                                                                          \
                                                                               \
                                                                               \
    std::ostream& operator<< (std::ostream& os, NAME##Message const& msg) {    \
        using namespace boost::fusion;                                         \
                                                                               \
        os << msg.type_name () << ": ";                                        \
        os << tuple_open ('[') << tuple_close (']') << tuple_delimiter (", "); \
        boost::fusion::operator<< (os, zip_names_and_values (msg.args ()));    \
                                                                               \
        return os;                                                             \
    }                                                                          \
                                                                               \
                                                                               \
    char const* const NAME##Message::type_name_ = #NAME;                       \
    char const* const NAME##Message::type_tag_  = TAG;


namespace synergy {
namespace protocol {
namespace v1 {

using boost::fusion::operator<< ;

MessageImpl::~MessageImpl () noexcept {
}

DEFINE_SYNERGY_MSG (Hello, "Synergy")

DEFINE_SYNERGY_MSG (HelloBack, "Synergy")

DEFINE_SYNERGY_MSG (Noop, "CNOP")

DEFINE_SYNERGY_MSG (Close, "CBYE")

DEFINE_SYNERGY_MSG (Enter, "CINN")

DEFINE_SYNERGY_MSG (Leave, "COUT")

DEFINE_SYNERGY_MSG (ClipboardGrab, "CCLP")

DEFINE_SYNERGY_MSG (Screensaver, "CSEC")

DEFINE_SYNERGY_MSG (ResetOptions, "CROP")

DEFINE_SYNERGY_MSG (InfoAck, "CIAK")

DEFINE_SYNERGY_MSG (KeepAlive, "CALV")

DEFINE_SYNERGY_MSG (KeyDown, "DKDN")

DEFINE_SYNERGY_MSG (KeyRepeat, "DKRP")

DEFINE_SYNERGY_MSG (KeyUp, "DKUP")

DEFINE_SYNERGY_MSG (MouseDown, "DMDN")

DEFINE_SYNERGY_MSG (MouseUp, "DMUP")

DEFINE_SYNERGY_MSG (MouseMove, "DMMV")

DEFINE_SYNERGY_MSG (MouseRelMove, "DMRM")

DEFINE_SYNERGY_MSG (MouseWheelMove, "DMWM")

DEFINE_SYNERGY_MSG (ClipboardData, "DCLP")

DEFINE_SYNERGY_MSG (Info, "DINF")

DEFINE_SYNERGY_MSG (SetOptions, "DSOP")

DEFINE_SYNERGY_MSG (FileTransfer, "DFTR")

DEFINE_SYNERGY_MSG (DragInfo, "DDRG")

DEFINE_SYNERGY_MSG (InfoQuery, "QINF")

DEFINE_SYNERGY_MSG (Incompatible, "EICV")

DEFINE_SYNERGY_MSG (Busy, "EBSY")

DEFINE_SYNERGY_MSG (Unknown, "EUNK")

DEFINE_SYNERGY_MSG (Bad, "EBAD")

} // namespace v1
} // namespace protocol
} // namespace synergy

#undef UNLIKELY
