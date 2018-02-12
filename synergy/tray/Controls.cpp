#include <synergy/tray/Controls.h>
#include <boost/asio/io_service.hpp>
#include <synergy/common/WampClient.h>
#include <thread>

class TrayControlsImpl final {
public:
    TrayControlsImpl (TrayControls* interface,
                      std::shared_ptr<spdlog::logger> log);
    void start();
    void shutdown();

private:
    TrayControls* interface;
    boost::asio::io_service ioService;

    std::thread rpcThread;
    WampClient rpcClient;
};

TrayControlsImpl::TrayControlsImpl (TrayControls* const ifc,
                                    std::shared_ptr<spdlog::logger> log):
    interface (interface),
    rpcClient (ioService, log) {
}

void
TrayControlsImpl::start() {
    rpcThread = std::thread([this](){
        rpcClient.start ("127.0.0.1", 24888);
        ioService.run();
    });
}

void
TrayControlsImpl::shutdown() {
    ioService.stop();
    rpcThread.join();
}

TrayControls::TrayControls ():
    m_impl (std::make_unique<TrayControlsImpl>(this, commonLog())) {
    m_impl->start();
}

TrayControls::~TrayControls() {
    m_impl->shutdown();
}
