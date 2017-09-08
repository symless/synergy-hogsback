#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

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

    void report(int screenId, const std::string& successfulIp, const std::string& failedIp);

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(std::string)> websocketMessageReceived;

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<UserConfig> m_userConfig;
    WebsocketSession m_websocket;
};

#endif // CLOUDCLIENT_H
