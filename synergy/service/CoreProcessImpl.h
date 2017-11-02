#pragma once

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/signals2.hpp>
#include <boost/process.hpp>
#include <synergy/common/ScreenStatus.h>

class CoreProcess;

class CoreProcessImpl {
public:
    CoreProcessImpl (
        CoreProcess& parent,
        boost::asio::io_service& io,
        std::vector<std::string> command
    );

    ~CoreProcessImpl() noexcept;
    void start();
    void shutdown();
    void onScreenStatusChanged (std::string screenName, ScreenStatus status);
    boost::asio::io_service& getIoService();

public:
    std::map<std::string, ScreenStatus> m_screenStates;
    std::vector<boost::signals2::connection> m_signals;
    std::vector<std::string> m_command;
    bool m_expectingExit = false;

    boost::asio::steady_timer m_connectionTimer;
    boost::asio::streambuf m_outputBuffer;
    boost::asio::streambuf m_errorBuffer;
    std::string m_lineBuffer;

    boost::process::async_pipe m_outPipe;
    boost::process::async_pipe m_errorPipe;
    boost::optional<boost::process::child> m_process;

private:
    CoreProcess& m_interface;
};
