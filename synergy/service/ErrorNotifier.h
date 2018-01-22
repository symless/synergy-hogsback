#pragma once

#include <boost/signals2.hpp>

class ErrorNotifier final {
public:
    explicit ErrorNotifier();

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

private:
};
