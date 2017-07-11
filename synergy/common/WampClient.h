#ifndef WAMPCLIENT_H
#define WAMPCLIENT_H

#include <boost/asio.hpp>
#include <string>

class WampClient
{
public:
    WampClient(std::string ip, int port, bool debug = false);

private:
    boost::asio::io_service m_ioService;
};

#endif // WAMPCLIENT_H
