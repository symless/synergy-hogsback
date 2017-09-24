#include <synergy/service/ServiceLogs.h>

#include <iostream>

using namespace boost;

Log g_commonLog;

class LogSignalSink : public spdlog::sinks::sink
{
    void log(const spdlog::details::log_msg& logLine) override
    {
        g_commonLog.onLogLine(logLine.formatted.str());
    }

    void flush() { }
};

static auto
initCommonLog()
{
    std::vector<spdlog::sink_ptr> sinks;

#ifdef _WIN32
    auto console = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
#else // assume unix (linux and mac)
    auto console = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif
    sinks.push_back(console);

    auto signal = std::make_shared<LogSignalSink>();
    sinks.push_back(signal);

    auto logger = std::make_shared<spdlog::logger>("main", begin(sinks), end(sinks));
    logger->set_pattern("[%Y-%m-%dT%T] %l: %v");
    logger->flush_on(spdlog::level::debug);
    logger->set_level(spdlog::level::debug);

    return logger;
}

std::shared_ptr<spdlog::logger> const&
commonLog() noexcept
{
    static auto log = initCommonLog();
    return log;
}

