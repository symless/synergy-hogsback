#pragma once

#include <boost/signals2.hpp>

class Router;

class RouterErrorMonitor final {
public:
    explicit RouterErrorMonitor();

    void monitor(Router& router);

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

private:
};
