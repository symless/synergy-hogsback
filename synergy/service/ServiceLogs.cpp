#include <synergy/service/ServiceLogs.h>

#include <synergy/common/DirectoryManager.h>

#include <iostream>

using namespace boost;

Log g_serviceLog;

class LogSignalSink : public spdlog::sinks::sink
{
    void log(const spdlog::details::log_msg& logLine) override
    {
        g_serviceLog.onLogLine(logLine.formatted.str());
    }

    void flush() { }
};

static auto
initServiceLog()
{
    std::vector<spdlog::sink_ptr> sinks;

#ifdef _WIN32
    auto console = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
#else // assume unix (linux and mac)
    auto console = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif
    sinks.push_back(console);

    auto logDir = DirectoryManager::instance()->systemLogDir();
    optional<filesystem::path> logPath;

    /* attempt to create log dir, report error later when logging has been
     * setup. apparently create_directory only returns false if the dir
     * doesn't exist (otherwise it throws). bah */
    std::string logDirExString;
    try {
        if (!is_directory(logDir)) {
            if (!create_directory(logDir)) {
                throw std::runtime_error("directory already exists: " + logDir.string());
            }
        }

        logPath = logDir / "synergy-service.log";
        auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logPath->string(), 1024 * 1024, 1);
        sinks.push_back(rotating);
    }
    catch (const std::exception& ex) {
        logDirExString = ex.what();
    }

    auto signal = std::make_shared<LogSignalSink>();
    sinks.push_back(signal);

    auto logger = std::make_shared<spdlog::logger>("main", begin(sinks), end(sinks));
    logger->set_pattern("[%Y-%m-%dT%T] %l: %v");
    logger->flush_on(spdlog::level::debug);
    logger->set_level(spdlog::level::debug);

    if (logPath) {
        logger->debug("service log path: {}", logPath->string());
    }
    else if (!logDirExString.empty()) {
        logger->warn("service file logging disabled, exception: {}", logDirExString);
    }

    return logger;
}

std::shared_ptr<spdlog::logger> const&
serviceLog() noexcept
{
    static auto log = initServiceLog();
    return log;
}
