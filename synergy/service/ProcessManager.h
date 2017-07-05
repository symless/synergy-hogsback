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
    explicit ProcessManager (std::shared_ptr<asio::io_service> io
                                = std::make_shared<asio::io_service>());

    ~ProcessManager() noexcept;
    ProcessManager (ProcessManager const&) = delete;
    ProcessManager& operator= (ProcessManager const&) = delete;

    void start (std::vector<std::string> command);
    bool awaitingExit() const noexcept;

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void()> onUnexpectedExit;
    signal<void()> onExit;
    signal<void(std::string, bool)> onOutput;

private:
    std::shared_ptr<asio::io_service> m_io;
    std::unique_ptr<ProcessManagerImpl> m_impl;
};

#endif // SYNERGY_SERVICE_PROCESSMANAGER_H
