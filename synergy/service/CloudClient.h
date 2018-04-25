#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include <synergy/service/WebsocketSession.h>
#include <synergy/service/WebsocketError.h>
#include <synergy/service/PriorityJobQueues.h>
#include <synergy/common/Screen.h>

#include <boost/beast/http.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>
#include <memory.h>

class UserConfig;
class HttpSession;
class ProfileConfig;
class Screen;

namespace http = boost::beast::http;

class CloudClient
{
public:
    CloudClient (boost::asio::io_service& ioService,
                 std::shared_ptr<UserConfig> userConfig,
                 std::shared_ptr<ProfileConfig> remoteProfileConfig);

    ~CloudClient();
    CloudClient(const CloudClient&) = default;
    CloudClient& operator=(const CloudClient&) = default;
    CloudClient(CloudClient&&) = default;
    CloudClient& operator=(CloudClient&&) = default;

    void claimServer(int64_t serverId);
    void updateScreenIpList(Screen& screen);
    void updateScreenStatus(Screen& screen);
    void updateScreenError(Screen& screen);
    void reconnectWebsocket();
    void shutdownWebsocket();
    bool isWebsocketConnected() const;
    bool setProxy (std::string const& protocol, std::string const& config);

private:
    struct HttpJob {
        std::string target;
        http::verb method;
        std::string context;
        int retryTimes = 0;
    };

    enum JobCategories {
        ProfileUpdate,
        ScreenUpdate,
        ScreenStatusUpdate,
        MaximumSize
    };

    using HTTPPriorityJobQueues = PriorityJobQueues <HttpJob>;

private:
    void load(const UserConfig &userConfig);
    void addHttpJob(JobCategories cat, HttpJob& job);
    void sendNextHttp();

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
    std::string m_httpProxy;
    int m_httpProxyPort = 80;
    HTTPPriorityJobQueues m_httpJobQueues;
    std::unique_ptr<HttpSession> m_httpSession;
};

#endif // CLOUDCLIENT_H
