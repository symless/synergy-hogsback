#ifndef SYNERGY_SERVICE_PROCESSMANAGER_H
#define SYNERGY_SERVICE_PROCESSMANAGER_H

#include <string>
#include <memory>
#include <synergy/service/IOService.h>
#include <synergy/common/Signals.h>
#include <vector>
#include <string>

class ProcessManagerImpl;

class ProcessManager final
{
public:
    explicit ProcessManager (asio::io_service& io);
    ~ProcessManager() noexcept;
    ProcessManager (ProcessManager const&) = delete;
    ProcessManager& operator= (ProcessManager const&) = delete;

    void start (std::vector<std::string> command);
    bool awaitingExit() const noexcept;

    auto& ioService() const noexcept { return m_mainIoService; }

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void()> onExit;
    signal<void()> onUnexpectedExit;
    signal<void(std::string)> onOutput;

private:
    asio::io_service& m_mainIoService;
    std::unique_ptr<ProcessManagerImpl> m_impl;
};

#endif // SYNERGY_SERVICE_PROCESSMANAGER_H
