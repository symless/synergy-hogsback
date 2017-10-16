#include <boost/mpl/at.hpp>
#include <boost/mpl/size.hpp>
#include <boost/variant/variant.hpp>
#include <type_traits>

namespace {

template <size_t N>
struct make_variant_of_nth_helper {
    template <typename Variant, typename Transform>
    Variant
    emplace (int const n, bool* const success, Transform&& transform) {
        using type_vector = typename Variant::types;
        using nth_type    = typename boost::mpl::at_c<type_vector, N>::type;

        if (n == N) {
            *success = true;
            return Variant (transform (nth_type ()));
        } else {
            return make_variant_of_nth_helper<N - 1> ()
                .template emplace<Variant> (
                    n, success, std::forward<Transform> (transform));
        }
    }
};

template <>
struct make_variant_of_nth_helper<0> {
    template <typename Variant, typename Transform>
    Variant
    emplace (int const n, bool* const success, Transform&& transform) {
        using type_vector = typename Variant::types;
        using nth_type    = typename boost::mpl::at_c<type_vector, 0>::type;
        *success = true;
        return Variant (transform (nth_type ()));
    }
};

template <typename Variant, typename Transform>
inline Variant
make_variant_of_nth (int n, bool* const success, Transform&& transform) {
    using type_vector = typename Variant::types;
    auto const max    = boost::mpl::size<type_vector>::value - 1;

    if ((n < 0) || (n > max)) {
        n = 0;
    }

    make_variant_of_nth_helper<boost::mpl::size<type_vector>::value - 1> next;
    return next.template emplace<Variant> (
        n, success, std::forward<Transform> (transform));
}

} // namespace
