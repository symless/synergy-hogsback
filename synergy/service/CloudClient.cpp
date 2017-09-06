#include <synergy/service/CloudClient.h>
#include <synergy/common/UserConfig.h>

static const char* const kPubSubServerHostname = "165.227.29.181";
static const char* const kPubSubServerPort = "8081";
static const char* const kSubTarget = "/synergy/sub/auth";
static const char* const kCloudServerHostname = "192.168.3.93";
static const char* const kCloudServerPort = "80";

CloudClient::CloudClient(boost::asio::io_service& ioService,
                         std::shared_ptr<UserConfig> userConfig) :
    m_ioService(ioService),
    m_httpSession(ioService, kCloudServerHostname, kCloudServerPort),
    m_userConfig(userConfig),
    m_websocket(ioService, kPubSubServerHostname, kPubSubServerPort)
{
    m_websocket.messageReceived.connect([this](std::string msg) {
        websocketMessageReceived(std::move(msg));
    });
}

void CloudClient::init()
{
    auto const profileId = m_userConfig->profileId();
    if (profileId != -1) {
        m_websocket.addHeader("X-Channel-Id", std::to_string(profileId));
        m_websocket.addHeader("X-Auth-Token", m_userConfig->userToken());

        if (!m_websocket.isConnected()) {
            m_websocket.connect(kSubTarget);
        }
    }
}
