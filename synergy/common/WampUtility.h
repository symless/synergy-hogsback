#ifndef WAMPUTILITY_H
#define WAMPUTILITY_H

#include <tuple>

namespace {

struct make_tuple {
    template <typename... Args>
    auto operator()(Args&&... args) const {
        return std::make_tuple (std::forward<Args>(args)...);
    }
};
}

#endif // WAMPUTILITY_H
