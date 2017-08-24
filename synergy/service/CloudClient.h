#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include "synergy/service/HttpSession.h"
#include "synergy/service/WebsocketSession.h"

#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>
#include <memory.h>

class UserConfig;

class CloudClient
{
public:
    CloudClient(boost::asio::io_service& ioService, std::shared_ptr<UserConfig> userConfig);

    void init();

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(std::string)> websocketMessageReceived;

private:
    boost::asio::io_service& m_ioService;
    HttpSession m_httpSession;
    std::shared_ptr<UserConfig> m_userConfig;
    WebsocketSession m_websocket;
};

#endif // CLOUDCLIENT_H
