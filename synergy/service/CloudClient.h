#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include "synergy/service/HttpSession.h"
#include "synergy/service/WebsocketSession.h"

#include <boost/asio/io_service.hpp>

class CloudClient
{
public:
    CloudClient(boost::asio::io_service& ioService);

    void init();

private:
    boost::asio::io_service& m_ioService;
    HttpSession m_httpSession;
    WebsocketSession m_websocket;
};

#endif // CLOUDCLIENT_H
