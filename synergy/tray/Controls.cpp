#include <synergy/tray/Controls.h>
#include <synergy/common/WampClient.h>
#include <synergy/common/DirectoryManager.h>
#include <synergy/common/RpcLogSink.h>
#include <boost/asio/io_service.hpp>
#include <spdlog/sinks/sink.h>
#include <thread>

static auto const kTrayLogPattern = "[ Tray    ] [%Y-%m-%dT%T] %l: %v";
static auto const kTrayLogLevel = spdlog::level::debug;
static auto const kTrayLogFlushLevel = spdlog::level::debug;

static auto
initTrayLogFileSink() {
    auto const logDir = DirectoryManager::instance()->systemLogDir();
    auto const logPath = logDir / "synergy-tray.log";

    return std::make_shared<spdlog::sinks::rotating_file_sink_mt>
                            (logPath.string(), 1024 * 1024, 1);
}

class TrayControlsImpl final {
public:
    explicit TrayControlsImpl (TrayControls*);
    ~TrayControlsImpl();

    void start();
    void shutdown();
    std::shared_ptr<spdlog::logger> log() const;

private:
    std::shared_ptr<spdlog::logger> fileLogger();
    std::shared_ptr<spdlog::logger> rpcLogger();

    TrayControls* interface = nullptr;
    std::shared_ptr<spdlog::sinks::sink> logFileSink;
    std::shared_ptr<spdlog::logger> logger;

    boost::asio::io_service ioService;
    std::thread rpcThread;
    WampClient rpcClient;
};

TrayControlsImpl::TrayControlsImpl (TrayControls* const ifc):
    interface (ifc),
    logFileSink (initTrayLogFileSink()),
    logger (fileLogger()),
    rpcClient (ioService, fileLogger()) {

    /* When the RPC is up, switch to the dual file/RPC logger */
    rpcClient.connected.connect ([&](){
        this->logger = rpcLogger();

        rpcClient.call<bool>("synergy.tray.hello")
            .then (rpcClient.executor(), [this](boost::future<bool> kill) {
            if (kill.get()) {
                log()->info ("Received kill command.");
                this->shutdown();
            }
        });
    });

    /* When the RPC goes down, switch to the file logger */
    rpcClient.disconnected.connect ([&](){
        this->logger = fileLogger();
    });
}

TrayControlsImpl::~TrayControlsImpl() {
    rpcThread.join();
}

void
TrayControlsImpl::start() {
    rpcThread = std::thread([this](){
        rpcClient.connect ("127.0.0.1", 24888);
        try {
            ioService.run();
        } catch (std::exception const&) {
        }
    });
}

void
TrayControlsImpl::shutdown() {
    ioService.dispatch ([this](){
        rpcClient.call<bool>("synergy.tray.goodbye");
        rpcClient.disconnect();
    });
}

std::shared_ptr<spdlog::logger>
TrayControlsImpl::log() const {
    return this->logger;
}

std::shared_ptr<spdlog::logger>
TrayControlsImpl::fileLogger() {
    std::vector<spdlog::sink_ptr> sinks = { logFileSink };

    auto logger = std::make_shared<spdlog::logger>("tray",
                                                   begin(sinks), end(sinks));
    logger->set_pattern (kTrayLogPattern);
    logger->set_level (kTrayLogLevel);
    logger->flush_on (kTrayLogFlushLevel);
    return logger;
}

std::shared_ptr<spdlog::logger>
TrayControlsImpl::rpcLogger() {
    std::vector<spdlog::sink_ptr> sinks = {
        logFileSink,
        std::make_shared<RpcLogSink>(rpcClient, "tray")
    };

    auto logger = std::make_shared<spdlog::logger>("tray",
                                                   begin(sinks), end(sinks));
    logger->set_pattern (kTrayLogPattern);
    logger->set_level (kTrayLogLevel);
    logger->flush_on (kTrayLogFlushLevel);
    return logger;
}

TrayControls::TrayControls():
    m_impl (std::make_unique<TrayControlsImpl>(this)) {
    m_impl->start();
}

TrayControls::~TrayControls() {
    m_impl->shutdown();
}

std::shared_ptr<spdlog::logger>
TrayControls::log() const {
    return m_impl->log();
}
