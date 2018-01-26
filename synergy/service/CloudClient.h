#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include "synergy/service/WebsocketSession.h"
#include "synergy/service/WebsocketError.h"
#include "synergy/common/Screen.h"

#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>
#include <memory.h>

class UserConfig;
class HttpSession;
class ProfileConfig;

class CloudClient
{
public:
    CloudClient (boost::asio::io_service& ioService,
                 std::shared_ptr<UserConfig> userConfig,
                 std::shared_ptr<ProfileConfig> remoteProfileConfig);

    void claimServer(int64_t serverId);
    void updateScreen(Screen& screen);
    void updateScreenError(Screen& screen);
    void reconnectWebsocket();
    bool isWebsocketConnected() const;
    void fakeScreenStatusUpdate();

private:
    void load(const UserConfig &userConfig);
    HttpSession* newHttpSession();
    static std::string pubSubServerHostname();
    static std::string cloudServerHostname();

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(std::string)> websocketMessageReceived;
    signal<void()> websocketConnected;
    signal<void()> websocketDisconnected;
    signal<void(WebsocketError)> websocketError;

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<UserConfig> m_userConfig;
    WebsocketSession m_websocket;
    std::shared_ptr<ProfileConfig> m_remoteProfileConfig;
    int m_lastProfileId = -1;
    std::string m_lastUserToken = "";
};

#endif // CLOUDCLIENT_H
