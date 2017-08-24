#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include "synergy/service/HttpSession.h"
#include "synergy/service/WebsocketSession.h"

#include <boost/asio/io_service.hpp>
#include <memory.h>

class UserConfig;

class CloudClient
{
public:
    CloudClient(boost::asio::io_service& ioService, std::shared_ptr<UserConfig> userConfig);

    void init();

private:
    boost::asio::io_service& m_ioService;
    HttpSession m_httpSession;
    std::shared_ptr<UserConfig> m_userConfig;
    WebsocketSession m_websocket;
};

#endif // CLOUDCLIENT_H
