#include <synergy/service/CloudClient.h>

#include <synergy/service/ServiceLogs.h>
#include <synergy/common/ProfileConfig.h>
#include <synergy/service/HttpSession.h>
#include <synergy/common/UserConfig.h>
#include <synergy/service/App.h>

#include <tao/json.hpp>

static const char* kLiveCloudServerHostname = "v1.api.cloud.symless.com";
static const char* kLivePubSubServerHostname = "pubsub1.cloud.symless.com";
static const char* kTestCloudServerHostname = "test-api.cloud.symless.com";
static const char* kTestPubSubServerHostname = "test-pubsub.cloud.symless.com";
static const char* kCloudServerPort = "443";
static const char* kPubSubServerPort = "443";
static const char* kSubTarget = "/synergy/sub/auth";

CloudClient::CloudClient(boost::asio::io_service& ioService,
                         std::shared_ptr<UserConfig> userConfig,
                         std::shared_ptr<ProfileConfig> remoteProfileConfig) :
    m_ioService(ioService),
    m_userConfig (std::move(userConfig)),
    m_websocket(ioService, pubSubServerHostname(), kPubSubServerPort),
    m_remoteProfileConfig(remoteProfileConfig)
{
    m_userConfig->updated.connect ([this]() { this->load (*m_userConfig); });
    load (*m_userConfig);

    m_remoteProfileConfig->profileServerChanged.connect(
        [this](int64_t serverId) {
            claimServer(serverId);
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

    m_websocket.connectionError.connect([this](WebsocketError error) {
        websocketError(error);
    });
}

void
CloudClient::load(UserConfig const& userConfig)
{
    auto const profileId = userConfig.profileId();
    auto const userToken = userConfig.userToken();

    if ((profileId != m_lastProfileId) || (userToken != m_lastUserToken)) {

        auto versionCheck = userConfig.versionCheck();
        auto versionString = SYNERGY_VERSION_STRING;

        serviceLog()->debug("setting websocket headers, channel={} auth={} version={} check={}",
                            profileId, userToken, versionString, versionCheck);

        m_websocket.addHeader("X-Channel-Id", std::to_string(profileId));
        m_websocket.addHeader("X-Auth-Token", userToken);
        m_websocket.addHeader("X-Synergy-Version", versionString);

        if (!versionCheck) {
            m_websocket.addHeader("X-Version-Check", "False");
        }

        if (!m_websocket.isConnected()) {
            serviceLog()->debug("initial websocket connection");
            m_websocket.connect(kSubTarget);
        }
        else {
            serviceLog()->debug("websocket reconnecting");
            m_websocket.reconnect();
        }
    }

    m_lastProfileId = profileId;
    m_lastUserToken = userToken;
}

void CloudClient::claimServer(int64_t serverId)
{
    boost::posix_time::ptime current = boost::posix_time::second_clock::local_time();
    static auto allowNextTime = current;
    static const auto kMinRequestInterval = boost::posix_time::seconds(1);
    if (current >= allowNextTime) {
        allowNextTime = current + kMinRequestInterval;
    }
    else {
        serviceLog()->warn("ignored sending claim server request, operation too frequent");
        return;
    }

    auto profileId = m_userConfig->profileId();
    serviceLog()->debug("sending claim server message, serverId={} profileId={}", serverId, profileId);

    static const std::string kUrlTarget = "/profile/server/claim";
    HttpSession* httpSession = newHttpSession();

    tao::json::value root;
    root["screen_id"] = serverId;
    root["profile_id"] = profileId;

    httpSession->post(kUrlTarget, tao::json::to_string(root));
}

void CloudClient::updateScreen(Screen& screen)
{
    static const std::string kUrlTarget = "/screen/update";
    HttpSession* httpSession = newHttpSession();

    tao::json::value root;
    root["id"] = screen.id();
    root["name"] = screen.name();
    root["status"] = screenStatusToString(screen.status());
    root["ipList"] = screen.ipList();
    root["version"] = screen.version();
    root["error_code"] = static_cast<uint32_t>(screen.errorCode());
    root["error_message"] = screen.errorMessage();

    httpSession->post(kUrlTarget, tao::json::to_string(root));
}

void
CloudClient::reconnectWebsocket()
{
    m_websocket.reconnect(true);
}

bool
CloudClient::isWebsocketConnected() const
{
    return m_websocket.isConnected();
}

void CloudClient::fakeScreenStatusUpdate()
{
    static const std::string kUrlTarget = "/dummy/screen/status/update";
    HttpSession* httpSession = newHttpSession();

    httpSession->get(kUrlTarget);
}

HttpSession* CloudClient::newHttpSession()
{
    // TODO: add lifetime management or make http session reusable
    HttpSession* httpSession = new HttpSession(m_ioService, cloudServerHostname(), kCloudServerPort);

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

std::string
CloudClient::pubSubServerHostname()
{
    if (App::options().count("use-test-cloud")) {
        return kTestPubSubServerHostname;
    }
    else {
        return kLivePubSubServerHostname;
    }
}

std::string
CloudClient::cloudServerHostname()
{
    if (App::options().count("use-test-cloud")) {
        return kTestCloudServerHostname;
    }
    else {
        return kLiveCloudServerHostname;
    }
}
