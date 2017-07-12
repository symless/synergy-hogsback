#ifndef WAMPSERVER_H
#define WAMPSERVER_H

#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>

class WampServer
{
public:
    WampServer();
    void start(boost::asio::io_service& io, std::string ip, int port, bool debug = true);

    boost::signals2::signal<void(std::vector<std::string>)> startCore;

private:
    boost::asio::io_service io;
};

#endif // WAMPSERVER_H
