#include <synergy/service/Logs.h>

#include <iostream>

static auto
initMainLog()
{
#ifdef _WIN32
    auto console = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
#else //ansi terminal colors
    auto console = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif

    auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "synergy-service.log", 1024 * 1024, 1);

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(console);
    sinks.push_back(rotating);

    auto logger = std::make_shared<spdlog::logger>("main", begin(sinks), end(sinks));
    logger->flush_on(spdlog::level::debug);
    logger->set_level(spdlog::level::debug);
    return logger;
}

std::shared_ptr<spdlog::logger> const&
mainLog() noexcept
{
    static auto log = initMainLog();
    return log;
}
