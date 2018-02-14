#include <synergy/service/ServiceLogs.h>
#include <synergy/common/DirectoryManager.h>

Log g_serviceLog;

class SignalLogSink : public spdlog::sinks::sink {
    void flush() {}

    void
    log (const spdlog::details::log_msg& logLine) override {
        g_serviceLog.onLogLine (logLine.formatted.str());
    }
};

static auto
initCombinedLogSink() {
    auto const logDir = DirectoryManager::instance()->systemLogDir();
    auto const logPath = logDir / "synergy-combined.log";

    return std::make_shared<spdlog::sinks::rotating_file_sink_mt>
                            (logPath.string(), 1024 * 1024, 1);
}

static auto
combinedLogSink() {
    static auto sink = initCombinedLogSink();
    return sink;
}

static auto
initServiceLog() {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back (combinedLogSink());

#ifdef _WIN32
    auto console = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
#else
    auto console = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif
    sinks.push_back (std::move (console));

    auto logDir = DirectoryManager::instance()->systemLogDir();
    auto logPath = logDir / "synergy-service.log";

    sinks.push_back (std::make_shared<spdlog::sinks::rotating_file_sink_mt>
                        (logPath.string(), 1024 * 1024, 1));
    sinks.push_back (std::make_shared<SignalLogSink>());

    auto logger = std::make_shared<spdlog::logger>("main",
                                                   begin(sinks), end(sinks));
    logger->set_pattern ("[ Service ] [%Y-%m-%dT%T] %l: %v");
    logger->flush_on (spdlog::level::debug);
    logger->set_level (spdlog::level::debug);
    logger->debug ("service log file path: {}", logPath.string());
    return logger;
}

static auto
initCoreLog() {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(combinedLogSink());

#ifdef _WIN32
    auto console = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
#else // assume unix (linux and mac)
    auto console = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif
    sinks.push_back(console);

    auto logger = std::make_shared<spdlog::logger>("main", begin(sinks), end(sinks));
    logger->set_pattern("[ Core    ] %v");
    logger->flush_on(spdlog::level::debug);
    logger->set_level(spdlog::level::debug);

    return logger;
}

static auto
initRouterLog() {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(combinedLogSink());

#ifdef _WIN32
    auto console = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
#else // assume unix (linux and mac)
    auto console = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif
    sinks.push_back(console);

    auto logDir = DirectoryManager::instance()->systemLogDir();
    auto logPath = logDir / "synergy-router.log";
    auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        logPath.string(), 1024 * 1024, 1);
    sinks.push_back(rotating);

    auto signal = std::make_shared<SignalLogSink>();
    sinks.push_back(signal);

    auto logger = std::make_shared<spdlog::logger>("main", begin(sinks), end(sinks));
    logger->set_pattern("[ Router  ] [%Y-%m-%dT%T] %l: %v");
    logger->flush_on(spdlog::level::debug);
    logger->set_level(spdlog::level::debug);

    return logger;
}

static auto
initConfigLog() {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(combinedLogSink());

#ifdef _WIN32
    auto console = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
#else // assume unix (linux and mac)
    auto console = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif
    sinks.push_back(console);

    auto logDir = DirectoryManager::instance()->systemLogDir();
    auto logPath = logDir / "synergy-config.log";
    auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        logPath.string(), 1024 * 1024, 1);
    sinks.push_back(rotating);

    auto logger = std::make_shared<spdlog::logger>("main", begin(sinks), end(sinks));
    logger->set_pattern("[ Config  ] %v");
    logger->flush_on(spdlog::level::debug);
    logger->set_level(spdlog::level::debug);

    return logger;
}

std::shared_ptr<spdlog::logger> const&
serviceLog() noexcept {
    static auto log = initServiceLog();
    return log;
}

std::shared_ptr<spdlog::logger> const&
coreLog() noexcept {
    static auto log = initCoreLog();
    return log;
}

std::shared_ptr<spdlog::logger> const&
routerLog() noexcept {
    static auto log = initRouterLog();
    return log;
}

std::shared_ptr<spdlog::logger> const&
configLog() noexcept {
    static auto log = initConfigLog();
    return log;
}
