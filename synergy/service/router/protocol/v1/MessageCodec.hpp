#include <boost/endian/conversion.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef _MSC_VER
#define UNLIKELY(X) X
#else
#define UNLIKELY(X) __builtin_expect (!!(X), 0)
#endif

namespace synergy {
namespace protocol {
namespace v1 {
namespace codec {

using boost::endian::big_to_native_inplace;
using boost::endian::native_to_big_inplace;


/* Traits to handle static and dynamically sized fields.
 */
template <typename T, typename Enable = void>
struct field_traits;

template <typename T>
struct field_traits<T, std::enable_if_t<std::is_integral<T>::value &&
                                        !std::is_same<T, bool>::value>> {
    static constexpr bool const is_fixed_size = true;
    static constexpr std::size_t
    size () noexcept {
        return sizeof (T);
    }
};

template <>
struct field_traits<std::string> {
    static constexpr bool const is_fixed_size = false;
    static std::size_t
    size_of (std::string const& str) noexcept {
        return 4 + str.size ();
    }
};

template <typename T>
inline std::enable_if_t<field_traits<T>::is_fixed_size, std::size_t>
size_of (T const&) noexcept {
    return field_traits<T>::size ();
}

template <typename T>
inline std::enable_if_t<!field_traits<T>::is_fixed_size, std::size_t>
size_of (T const& value) noexcept {
    return field_traits<T>::size_of (value);
}


/* Integers
 */
template <typename T>
inline std::enable_if_t<std::is_integral<T>::value &&
                        !std::is_same<T, bool>::value>
read_from (char const*& buf, std::size_t& bufsize, T& value) {
    if (UNLIKELY (bufsize < sizeof (value))) {
        throw std::runtime_error (
            "Integer field extends beyond message buffer bounds");
    }

    std::memcpy (&value, buf, sizeof (value));
    big_to_native_inplace (value);

    buf += sizeof (value);
    bufsize -= sizeof (value);
}

template <typename T>
inline std::enable_if_t<std::is_integral<T>::value &&
                        !std::is_same<T, bool>::value>
write_to (char*& buf, T value) noexcept {
    native_to_big_inplace (value);
    std::memcpy (buf, &value, sizeof (value));
    buf += sizeof (value);
}


/* Strings
 */
template <typename T>
inline std::enable_if_t<std::is_same<T, std::string>::value>
read_from (char const*& buf, std::size_t& bufsize, T& str) {
    std::uint32_t size;
    read_from (buf, bufsize, size);
    if (UNLIKELY (bufsize < size)) {
        throw std::runtime_error (
            "String extends beyond message buffer bounds");
    }
    str.assign (buf, size);
    buf += size;
    bufsize -= size;
}

template <typename T>
inline std::enable_if_t<std::is_same<T, std::string>::value>
write_to (char*& buf, T const& str) {
    write_to (buf, boost::numeric_cast<std::uint32_t> (str.size ()));
    std::memcpy (buf, str.data (), str.size ());
    buf += str.size ();
}


/* Fusion Sequences
 */
template <typename T>
inline std::enable_if_t<boost::fusion::traits::is_sequence<T>::value,
                        std::size_t>
size_of (T const& sequence) noexcept {
    std::size_t n = 0;
    boost::fusion::for_each (sequence,
                             [&n](auto const& obj) { n += size_of (obj); });
    return n;
}

template <typename T>
inline std::enable_if_t<boost::fusion::traits::is_sequence<T>::value>
read_from (char const*& buf, std::size_t& bufsize, T& seq) {
    boost::fusion::for_each (
        seq, [&buf, &bufsize](auto& obj) { read_from (buf, bufsize, obj); });
}

template <typename T>
inline std::enable_if_t<boost::fusion::traits::is_sequence<T>::value>
write_to (char*& buf, T const& seq) noexcept {
    boost::fusion::for_each (seq,
                             [&buf](auto const& obj) { write_to (buf, obj); });
}


/* Vectors
 */
template <typename T>
inline std::enable_if_t<field_traits<T>::is_fixed_size, std::size_t>
size_of (std::vector<T> const& v) noexcept {
    std::size_t n = 4;
    n += v.size () * field_traits<T>::size ();
    return n;
}

template <typename T>
inline std::enable_if_t<field_traits<T>::is_fixed_size, void>
read_from (char const*& buf, std::size_t& bufsize, std::vector<T>& vec) {
    uint32_t size;
    read_from (buf, bufsize, size);
    if (UNLIKELY (bufsize < (size * field_traits<T>::size ()))) {
        throw std::runtime_error ("Array extends beyond message buffer bounds");
    }
    vec.resize (size);
    for (auto& obj : vec) {
        read_from (buf, bufsize, obj);
    }
}

template <typename T>
inline void
write_to (char*& dst, std::vector<T> const& vec) {
    write_to (dst, boost::numeric_cast<std::uint32_t> (vec.size ()));
    for (auto const& obj : vec) {
        write_to (dst, obj);
    }
}

} // namespace codec
} // namespace v1
} // namespace protocol
} // namespace synergy

#undef UNLIKELY
