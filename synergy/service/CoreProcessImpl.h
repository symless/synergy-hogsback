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
    boost::asio::io_service& getIoService();
    void onScreenStatusChanged (std::string screenName, ScreenStatus status);

private:
    CoreProcess& m_interface;
    std::vector<std::string> m_command;
    boost::asio::streambuf m_outputBuffer;
    boost::asio::streambuf m_errorBuffer;
    std::string m_lineBuffer;

    boost::process::async_pipe m_outPipe;
    boost::process::async_pipe m_errorPipe;
    boost::optional<boost::process::child> m_process;
    boost::asio::steady_timer m_connectionTimer;

public:
    /* TODO: these should probably be moved back out to the interface class? */
    std::vector<boost::signals2::connection> m_signals;
    std::map<std::string, ScreenStatus> m_screenStates;

private:
    bool m_expectingExit = false;
};
