#pragma once

#include <synergy/common/Logs.h>

extern Log g_serviceLog;

std::shared_ptr<spdlog::logger> const& serviceLog() noexcept;
std::shared_ptr<spdlog::logger> const& routerLog() noexcept;
std::shared_ptr<spdlog::logger> const& coreLog() noexcept;
std::shared_ptr<spdlog::logger> const& configLog() noexcept;
