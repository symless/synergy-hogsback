#ifndef WAMPSERVER_H
#define WAMPSERVER_H

#include <boost/asio/io_service.hpp>

class WampServer
{
public:
    WampServer();
    void start(std::string ip, int port, bool debug = false);

private:
    boost::asio::io_service m_ioService;
};

#endif // WAMPSERVER_H
