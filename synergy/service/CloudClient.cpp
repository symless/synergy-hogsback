#include <synergy/service/CloudClient.h>

#include <synergy/service/HttpSession.h>
#include <synergy/common/UserConfig.h>

#include <tao/json.hpp>

static const char* kPubSubServerHostname = "165.227.29.181";
static const char* kPubSubServerPort = "8081";
static const char* kSubTarget = "/synergy/sub/auth";
static const char* kCloudServerHostname = "v1.api.cloud.symless.com";
static const char* kCloudServerPort = "443";

CloudClient::CloudClient(boost::asio::io_service& ioService, std::shared_ptr<UserConfig> userConfig) :
    m_ioService(ioService),
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

void CloudClient::report(int screenId, const std::string &successfulIp, const std::string &failedIp)
{
    static const std::string kUrlTarget = "/report";
    HttpSession* httpSession = new HttpSession(m_ioService, kCloudServerHostname, kCloudServerPort);

    tao::json::value root;
    root["src"] = m_userConfig->screenId();
    root["dest"] = screenId;
    root["successfulIpList"] = successfulIp;
    root["failedIpList"] = failedIp;

    httpSession->addHeader("X-Auth-Token", m_userConfig->userToken());

    httpSession->requestSuccess.connect(
        [](HttpSession* session, std::string){
        delete session;

    });
    httpSession->requestFailed.connect(
        [](HttpSession* session, std::string){
        delete session;
    });

    httpSession->post(kUrlTarget, tao::json::to_string(root));
}
