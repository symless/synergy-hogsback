#ifndef SYNERGY_SERVICE_PROCESSMANAGER_H
#define SYNERGY_SERVICE_PROCESSMANAGER_H

#include "synergy/common/ErrorMessage.h"
#include "synergy/common/ScreenStatus.h"

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include <string>
#include <memory>
#include <vector>
#include <string>

class ProcessManagerImpl;

class ProcessManager final {
public:
    explicit ProcessManager (boost::asio::io_service& io);
    ProcessManager (ProcessManager const&) = delete;
    ProcessManager& operator= (ProcessManager const&) = delete;
    ~ProcessManager() noexcept;

    void start (std::vector<std::string> command);
    void shutdown();

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void()> onExit;
    signal<void()> onUnexpectedExit;
    signal<void(std::string const&)> onOutput;
    signal<void(std::string const&, ScreenStatus)> screenStatusChanged;
    signal<void(std::string const&, ErrorCode)> screenConnectionError;
    signal<void()> localInputDetected;

private:
    boost::asio::io_service& m_ioService;
    std::unique_ptr<ProcessManagerImpl> m_impl;
};

#endif // SYNERGY_SERVICE_PROCESSMANAGER_H
