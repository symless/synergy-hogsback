#ifndef WAMPSERVER_H
#define WAMPSERVER_H

#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>
#include <autobahn/autobahn.hpp>
// NOTE: AsioExecutor has to be included after autobahn
#include <synergy/common/AsioExecutor.h>

class WampServer
{
public:
    WampServer(boost::asio::io_service& io, std::string ip, int port, bool debug = true);
    void start();
    void shutdown();

    boost::signals2::signal<void(std::vector<std::string>)> startCore;

private:
    std::shared_ptr<autobahn::wamp_session> m_session;
    std::shared_ptr<autobahn::wamp_tcp_transport> m_transport;
    boost::executors::executor_adaptor<AsioExecutor> m_executor;
};

#endif // WAMPSERVER_H
