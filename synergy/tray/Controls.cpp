#include <synergy/tray/Controls.h>
#include <synergy/common/WampClient.h>
#include <synergy/common/DirectoryManager.h>
#include <synergy/common/RpcLogSink.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/spawn.hpp>
#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/null_sink.h>
#include <thread>

static auto const kTrayLogPattern = "[ Tray    ] [%Y-%m-%dT%T] %l: %v";
static auto const kTrayLogLevel = spdlog::level::debug;
static auto const kTrayLogFlushLevel = spdlog::level::debug;
static auto const kTrayPingInterval = std::chrono::seconds (3);

class TrayControlsImpl final {
public:
    explicit TrayControlsImpl (TrayControls*);
    ~TrayControlsImpl();

    void start();
    void shutdown();
    std::shared_ptr<spdlog::logger> log() const;

private:
    void pingLoop (boost::asio::yield_context ctx);
    std::shared_ptr<spdlog::logger> consoleLogger();
    std::shared_ptr<spdlog::logger> fileLogger();
    std::shared_ptr<spdlog::logger> rpcLogger();

    static spdlog::sink_ptr logFileSink();
    static spdlog::sink_ptr initlogFileSink() noexcept;

private:
    TrayControls* m_interface = nullptr;
    std::shared_ptr<spdlog::logger> m_logger;

    boost::asio::io_service m_ioService;
    std::thread m_rpcThread;
    WampClient m_rpcClient;

    boost::asio::steady_timer m_pingTimer;
    bool m_active = false;
};

spdlog::sink_ptr
TrayControlsImpl::initlogFileSink() noexcept {
    try {
        auto const logDir = DirectoryManager::instance()->systemLogDir();
        auto const logPath = logDir / "synergy-tray.log";

        return std::make_shared<spdlog::sinks::rotating_file_sink_mt>
                    (logPath.string(), 1024 * 1024, 1);
    } catch (std::exception const&) {
        /* Opening the log file will fail if a tray is already open */
    } catch (...) {
    }

    return std::make_shared<spdlog::sinks::null_sink_mt>();
}

spdlog::sink_ptr
TrayControlsImpl::logFileSink() {
    static auto sink = initlogFileSink();
    return sink;
}

TrayControlsImpl::TrayControlsImpl (TrayControls* const interface):
    m_interface (interface),
    m_logger (fileLogger()),
    m_rpcClient (m_ioService, fileLogger()),
    m_pingTimer (m_ioService)
{
    /* When the RPC comes up, switch to the dual file/RPC logger and start
     * pinging */
    m_rpcClient.connected.connect ([&](){
        this->m_logger = rpcLogger();

        m_rpcClient.call<bool>("synergy.tray.hello")
            .then (m_rpcClient.executor(), [this](boost::future<bool> kill) {

            // A tray process is already running, shut everything down
            if (kill.get()) {
                log()->info ("Received kill command.");
                this->shutdown();
                return;
            }

            m_active = true;

            boost::asio::spawn (m_rpcClient.ioService(), [this](auto ctx) {
                this->pingLoop(ctx);
            });

            m_interface->ready();
        });
    });

    /* When the RPC goes down, switch to the file logger and stop pinging */
    m_rpcClient.disconnected.connect ([&](bool){
        this->m_logger = fileLogger();

        boost::system::error_code ec;
        this->m_pingTimer.cancel(ec);
        this->m_ioService.poll();
    });
}

TrayControlsImpl::~TrayControlsImpl() {
    m_rpcThread.join();
}

void
TrayControlsImpl::start() {
    m_rpcThread = std::thread([this]() {
        m_rpcClient.connect ("127.0.0.1", 24888);
        try {
            m_ioService.run();
        } catch (std::exception const& ex) {
            this->m_logger = fileLogger();
            log()->critical ("Exception: {}", ex.what());
        }
    });
}

void
TrayControlsImpl::shutdown() {
    m_ioService.dispatch ([this]() {
        boost::system::error_code ec;
        m_pingTimer.cancel(ec);

        if (!this->m_active) {
            m_rpcClient.disconnect();
            return;
        }

        m_rpcClient.call<void>("synergy.tray.goodbye").then
            (m_rpcClient.executor(), [this](boost::future<void> result) {
                result.get();
                this->m_active = false;
                m_rpcClient.disconnect();
            }
        );
    });
}

void
TrayControlsImpl::pingLoop (boost::asio::yield_context ctx) {
    boost::system::error_code ec;

    while (true) {
        m_pingTimer.expires_from_now (kTrayPingInterval);
        m_pingTimer.async_wait (ctx[ec]);

        if (ec == boost::asio::error::operation_aborted) {
            return;
        } else if (ec) {
            this->shutdown();
            throw boost::system::system_error (ec);
        }

        m_rpcClient.call<void>("synergy.tray.ping");
    }
}

std::shared_ptr<spdlog::logger>
TrayControlsImpl::log() const {
    return this->m_logger;
}

std::shared_ptr<spdlog::logger>
TrayControlsImpl::fileLogger() {
    std::vector<spdlog::sink_ptr> sinks = { logFileSink() };

    auto logger = std::make_shared<spdlog::logger>("tray",
                                                   begin(sinks), end(sinks));
    logger->set_pattern (kTrayLogPattern);
    logger->set_level (kTrayLogLevel);
    logger->flush_on (kTrayLogFlushLevel);

    return logger;
}

std::shared_ptr<spdlog::logger>
TrayControlsImpl::rpcLogger() {
    std::vector<spdlog::sink_ptr> sinks
        = { logFileSink(), std::make_shared<RpcLogSink>(m_rpcClient, "tray") };

    auto logger = std::make_shared<spdlog::logger>("tray",
                                                   begin(sinks), end(sinks));
    logger->set_pattern (kTrayLogPattern);
    logger->set_level (kTrayLogLevel);
    logger->flush_on (kTrayLogFlushLevel);

    return logger;
}

TrayControls::TrayControls():
    m_impl (std::make_unique<TrayControlsImpl>(this)) {
}

TrayControls::~TrayControls() {
    m_impl->shutdown();
}

void
TrayControls::connect()
{
    m_impl->start();
}

std::shared_ptr<spdlog::logger>
TrayControls::log() const {
    return m_impl->log();
}
