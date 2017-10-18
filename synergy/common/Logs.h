#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>

std::shared_ptr<spdlog::logger> const& commonLog() noexcept;

struct Log {
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(std::string)> onLogLine;
};

extern Log g_commonLog;
