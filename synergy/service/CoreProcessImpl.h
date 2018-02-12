#pragma once

#include <synergy/common/ScreenStatus.h>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <boost/process.hpp>

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

private:
    CoreProcess& m_interface;
    std::vector<std::string> m_command;
    boost::asio::streambuf m_outputBuffer;
    boost::asio::streambuf m_errorBuffer;
    std::string m_lineBuffer;

    boost::process::async_pipe m_outPipe;
    boost::process::async_pipe m_errorPipe;
    boost::optional<boost::process::child> m_process;

private:
    bool m_expectingExit = false;
    boost::asio::io_service& m_io;
};
