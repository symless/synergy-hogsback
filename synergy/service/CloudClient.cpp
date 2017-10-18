#include <synergy/service/CloudClient.h>

#include <synergy/service/ServiceLogs.h>
#include <synergy/common/ProfileConfig.h>
#include <synergy/service/HttpSession.h>
#include <synergy/common/UserConfig.h>

#include <tao/json.hpp>

static const char* kPubSubServerHostname = "pubsub1.cloud.symless.com";
static const char* kPubSubServerPort = "443";
static const char* kSubTarget = "/synergy/sub/auth";
static const char* kCloudServerHostname = "v1.api.cloud.symless.com";
static const char* kCloudServerPort = "443";

CloudClient::CloudClient(boost::asio::io_service& ioService,
                         std::shared_ptr<UserConfig> userConfig,
                         std::shared_ptr<ProfileConfig> remoteProfileConfig) :
    m_ioService(ioService),
    m_userConfig (std::move(userConfig)),
    m_websocket(ioService, kPubSubServerHostname, kPubSubServerPort),
    m_remoteProfileConfig(remoteProfileConfig)
{
    m_userConfig->updated.connect ([this]() { this->load (*m_userConfig); });
    load (*m_userConfig);

    m_remoteProfileConfig->profileServerChanged.connect([this](int64_t serverId){
        claimServer(serverId);
    });

    m_remoteProfileConfig->screenTestResultChanged.connect(
        [this](int64_t screenId, std::string successfulIp, std::string failedIp) {
            report(screenId, successfulIp, failedIp);
        }
    );

    m_websocket.messageReceived.connect([this](std::string msg) {
        websocketMessageReceived(std::move(msg));
    });

    m_websocket.connected.connect([this]() {
        websocketConnected();
    });

    m_websocket.disconnected.connect([this]() {
        websocketDisconnected();
    });

    m_websocket.connectionError.connect([this]() {
        websocketConnectionError();
    });
}

void
CloudClient::load(UserConfig const& userConfig)
{
    static auto lastProfileId = -1;
    auto const profileId = userConfig.profileId();
    if (profileId != lastProfileId) {
        m_websocket.addHeader("X-Channel-Id", std::to_string(profileId));
        m_websocket.addHeader("X-Auth-Token", userConfig.userToken());

        if (!m_websocket.isConnected()) {
            m_websocket.connect(kSubTarget);
        }
        else {
            m_websocket.reconnect();
        }
    }

    lastProfileId = profileId;
}

void CloudClient::report(int screenId, const std::string &successfulIp, const std::string &failedIp)
{
    static const std::string kUrlTarget = "/report";
    HttpSession* httpSession = newHttpSession();

    tao::json::value root;
    root["src"] = m_userConfig->screenId();
    root["dest"] = screenId;
    root["successfulIpList"] = successfulIp;
    root["failedIpList"] = failedIp;


    httpSession->post(kUrlTarget, tao::json::to_string(root));
}

void CloudClient::claimServer(int64_t serverId)
{
    auto profileId = m_userConfig->profileId();
    serviceLog()->debug("sending claim server message, serverId={} profileId={}", serverId, profileId);

    static const std::string kUrlTarget = "/profile/server/claim";
    HttpSession* httpSession = newHttpSession();

    tao::json::value root;
    root["screen_id"] = serverId;
    root["profile_id"] = profileId;

    httpSession->post(kUrlTarget, tao::json::to_string(root));
}

void
CloudClient::reconnectWebsocket()
{
    m_websocket.reconnect(true);
}

bool
CloudClient::isWebsocketConnected()
{
    return m_websocket.isConnected();
}

HttpSession* CloudClient::newHttpSession()
{
    // TODO: add lifetime management or make http session reusable
    HttpSession* httpSession = new HttpSession(m_ioService, kCloudServerHostname, kCloudServerPort);

    httpSession->addHeader("X-Auth-Token", m_userConfig->userToken());

    httpSession->requestSuccess.connect(
        [](HttpSession* session, std::string){
        delete session;

    });
    httpSession->requestFailed.connect(
        [](HttpSession* session, std::string){
        delete session;
    });

    return httpSession;
}
