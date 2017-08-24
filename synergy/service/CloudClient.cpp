#include "CloudClient.h"

#include <synergy/common/UserConfig.h>

//static const char* kPubSubServerHostname = "165.227.29.181";
static const char* kPubSubServerHostname = "192.168.3.93";
static const char* kPubSubServerPort = "80";
static const char* kSubTarget = "/sub/auth";
static const char* kCloudServerHostname = "192.168.3.93";
static const char* kCloudServerPort = "80";
CloudClient::CloudClient(boost::asio::io_service& ioService, std::shared_ptr<UserConfig> userConfig) :
    m_ioService(ioService),
    m_httpSession(ioService, kCloudServerHostname, kCloudServerPort),
    m_userConfig(userConfig),
    m_websocket(ioService, kPubSubServerHostname, kPubSubServerPort)
{
}

void CloudClient::init()
{
    int64_t profileId = 1;//m_userConfig->profileId();
    if (profileId != -1) {
        m_websocket.addHeader("X-Channel-Id", std::to_string(profileId));
        m_websocket.addHeader("X-Auth-Token", m_userConfig->userToken());

        if (m_websocket.isConnected()) {
            m_websocket.reconnect();
        }
        else {
            m_websocket.connect(kSubTarget);
        }
    }
}
