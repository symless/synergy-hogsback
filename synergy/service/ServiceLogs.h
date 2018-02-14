#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <boost/signals2.hpp>

struct Log {
    boost::signals2::signal<void(std::string)> onLogLine;
};

extern Log g_serviceLog;

std::shared_ptr<spdlog::logger> const& serviceLog() noexcept;
std::shared_ptr<spdlog::logger> const& routerLog() noexcept;
std::shared_ptr<spdlog::logger> const& coreLog() noexcept;
std::shared_ptr<spdlog::logger> const& configLog() noexcept;
std::shared_ptr<spdlog::logger> const& trayLog() noexcept;
