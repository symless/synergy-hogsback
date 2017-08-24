#ifndef FROM_JSON_HPP
#define FROM_JSON_HPP

#include <boost/fusion/adapted/mpl.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/sequence/intrinsic/at_c.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/optional.hpp>
#include <cmath>
#include <fmt/format.h>
#include <iostream>
#include <map>
#include <string>
#include <tao/json.hpp>
#include <type_traits>

namespace json = tao::json;

#define SCS_LIKELY(X) __builtin_expect (!!(X), 1)
#define SCS_UNLIKELY(X) __builtin_expect (!!(X), 0)

#define DEFINE_JSON(NAME, ...)                                                 \
    BOOST_FUSION_DEFINE_STRUCT ((), NAME, __VA_ARGS__);

template <typename T>
inline std::enable_if_t<boost::fusion::traits::is_sequence<T>::value>
from_json (T& dest, json::value& src, char const* const field_name = "");

inline void
from_json (bool& dest, json::value& src, char const* const field_name = "") {
    if (SCS_UNLIKELY (!src.is_boolean ())) {
        throw std::runtime_error (
            fmt::format ("Expected field '{}' to be a boolean, but found a "
                         "value of '{}' type",
                         field_name,
                         json::to_string (src.type ())));
    }
    dest = src.unsafe_get_boolean ();
}

inline void
from_json (std::string& dest, json::value& src,
           char const* const field_name = "") {
    if (SCS_UNLIKELY (!src.is_string ())) {
        throw std::runtime_error (
            fmt::format ("Expected field '{}' to be a string, but found a "
                         "value of '{}' type",
                         field_name,
                         json::to_string (src.type ())));
    }
    dest = std::move (src.unsafe_get_string ());
}

template <typename T>
inline void
from_json (std::vector<T>& dest, json::value& src,
           char const* const field_name = "") {
    if (SCS_UNLIKELY (!src.is_array ())) {
        throw std::runtime_error (
            fmt::format ("Expected field '{}' to be an array, but found a "
                         "value of '{}' type",
                         field_name,
                         json::to_string (src.type ())));
    }

    auto& json_array = src.get_array ();
    for (auto& element : json_array) {
        dest.emplace_back ();
        from_json (dest.back (), element);
    }
}

template <typename T>
inline std::enable_if_t<std::is_integral<T>::value &&
                        !std::is_same<bool, T>::value>
from_json (T& dest, json::value& src, char const* const field_name = "") {
    if (src.is_unsigned ()) {
        dest = boost::numeric_cast<T> (src.unsafe_get_unsigned ());
    } else if (src.is_signed ()) {
        dest = boost::numeric_cast<T> (src.unsafe_get_signed ());
    } else if (SCS_UNLIKELY (src.is_double ())) {
        auto d = src.unsafe_get_double ();
        if (SCS_UNLIKELY (modf (d, &d) != 0.0)) {
            throw std::range_error (fmt::format (
                "Expected field '{}' to be of integral type, but found a "
                "floating point value that requires rounding",
                field_name));
        }
        dest = boost::numeric_cast<T> (d);
    } else {
        throw std::runtime_error (
            fmt::format ("Expected field '{}' to be of integral type, but "
                         "found a value of '{}' type",
                         field_name,
                         json::to_string (src.type ())));
    }
}

template <typename T>
inline void
from_json (boost::optional<T>& dest, json::value* const src,
           char const* const field_name = "") {
    if (!src || src->is_null ()) {
        dest = boost::none;
        return;
    }
    dest.emplace ();
    return from_json (*dest, *src, field_name);
}

template <typename T>
inline void
from_json (T& dest, json::value* const src, char const* const field_name = "") {
    if (SCS_UNLIKELY (!src || src->is_null ())) {
        throw std::runtime_error (
            fmt::format ("Missing or null '{}' field", field_name));
    }
    return from_json (dest, *src, field_name);
}

template <typename T>
inline std::enable_if_t<boost::fusion::traits::is_sequence<T>::value>
from_json (T& dest, std::map<std::string, json::value>& src,
           char const* const = "") {

    using range = boost::mpl::
        range_c<int, 0, boost::fusion::result_of::size<T>::type::value>;

    boost::fusion::for_each (range (), [&dest, &src](auto i) {
        auto const field_name =
            boost::fusion::extension::struct_member_name<T, i>::call ();
        auto const it = src.find (field_name);
        auto& value   = boost::fusion::at_c<i> (dest);
        if (it == src.end ()) {
            return from_json (value, nullptr, field_name);
        }
        from_json (value, &it->second, field_name);
    });
}

template <typename T>
inline std::enable_if_t<boost::fusion::traits::is_sequence<T>::value>
from_json (T& dest, json::value& src, char const* const field_name) {
    from_json (dest, src.get_object (), field_name);
}

template <typename T>
inline void
from_json (T& dest, std::string const& str) {
    auto value = json::from_string (str);
    from_json (dest, value);
}

#undef SCS_LIKELY
#undef SCS_UNLIKELY

#endif
