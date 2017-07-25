#ifndef SYNERGY_SERVICE_PROCESSMANAGER_H
#define SYNERGY_SERVICE_PROCESSMANAGER_H

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include <string>
#include <memory>
#include <vector>
#include <string>

class ProcessManagerImpl;

class ProcessManager final
{
public:
    explicit ProcessManager (boost::asio::io_service& io);
    ~ProcessManager() noexcept;
    ProcessManager (ProcessManager const&) = delete;
    ProcessManager& operator= (ProcessManager const&) = delete;

    void start (std::vector<std::string> command);
    bool awaitingExit() const noexcept;
    void stop();

    auto& ioService() const noexcept { return m_ioService; }

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void()> onExit;
    signal<void()> onUnexpectedExit;
    signal<void(std::string)> onOutput;

private:
    boost::asio::io_service& m_ioService;
    std::unique_ptr<ProcessManagerImpl> m_impl;
};

#endif // SYNERGY_SERVICE_PROCESSMANAGER_H
