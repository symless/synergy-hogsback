#include <synergy/service/Logs.h>

#include <synergy/common/DirectoryManager.h>

#include <iostream>

Log g_log;

class LogSignalSink : public spdlog::sinks::sink
{
    void log(const spdlog::details::log_msg& logLine) override
    {
        g_log.onLogLine(logLine.formatted.str());
    }

    void flush() { }
};

static auto
initMainLog()
{
#ifdef _WIN32
    auto console = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
#else // assume unix (linux and mac)
    auto console = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif

    auto logDir = DirectoryManager::instance()->systemLogDir();
    auto logPath = logDir / "synergy-service.log";

    auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        logPath.string(), 1024 * 1024, 1);

    auto signal = std::make_shared<LogSignalSink>();

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(console);
    sinks.push_back(rotating);
    sinks.push_back(signal);

    auto logger = std::make_shared<spdlog::logger>("main", begin(sinks), end(sinks));
    logger->set_pattern("[%Y-%m-%dT%T] %l: %v");
    logger->flush_on(spdlog::level::debug);
    logger->set_level(spdlog::level::debug);

    logger->debug("service log path: {}", logPath.string());

    return logger;
}

std::shared_ptr<spdlog::logger> const&
mainLog() noexcept
{
    static auto log = initMainLog();
    return log;
}
