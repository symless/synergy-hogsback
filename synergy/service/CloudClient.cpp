#include <synergy/service/CloudClient.h>

#include <synergy/service/ServiceLogs.h>
#include <synergy/service/HttpSession.h>
#include <synergy/common/ProfileConfig.h>
#include <synergy/common/UserConfig.h>
#include <synergy/common/Screen.h>
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
    m_remoteProfileConfig(remoteProfileConfig),
    m_httpJobQueues(JobCategories::MaximumSize),
    m_httpSession(std::make_unique<HttpSession>(m_ioService, cloudServerHostname(), kCloudServerPort))
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

    m_httpSession->responseReceived.connect([this](http::status result, std::string response) {
        if (result != http::status::ok) {
            auto& lastJob = m_httpJobQueues.nextJob();
            serviceLog()->debug("invalid http request to {}: {}", lastJob.target, response);

            if (lastJob.retryTimes-- <= 0) {
                m_httpJobQueues.popJob();
            }
        }
        else {
            m_httpJobQueues.popJob();

        }

        sendNextHttp();
    });

    m_httpSession->requestFailed.connect([this](errorCode ec) {
        // without popping the top job, this means retry
        sendNextHttp();
    });
}

CloudClient::~CloudClient() {
}

void
CloudClient::load(UserConfig const& userConfig)
{
    auto const profileId = userConfig.profileId();
    auto const userToken = userConfig.userToken();

    if ((profileId != -1) && ((profileId != m_lastProfileId) || (userToken != m_lastUserToken))) {

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

    m_httpSession->addHeader("X-Auth-Token", m_userConfig->userToken());
}

void CloudClient::addHttpJob(CloudClient::JobCategories cat, CloudClient::HttpJob &job)
{
    m_httpJobQueues.appendJob(cat, job);

    if (m_httpJobQueues.size() == 1) {
        sendNextHttp();
    }
}

void CloudClient::sendNextHttp()
{
    if (m_httpJobQueues.hasJob()) {
        auto& nextJob = m_httpJobQueues.nextJob();

        m_httpSession->setupRequest(nextJob.method, nextJob.target, nextJob.context);
        m_httpSession->send();
    }
}

void CloudClient::claimServer(int64_t serverId)
{
    HttpJob job;

    auto profileId = m_userConfig->profileId();
    int64_t profileVersion = m_remoteProfileConfig->profileVersion();
    tao::json::value root;
    root["screen_id"] = serverId;
    root["profile_id"] = profileId;
    root["profile_version"] = profileVersion;

    job.target = "/profile/server/claim";
    job.method = http::verb::post;
    job.context = tao::json::to_string(root);

    addHttpJob(JobCategories::ProfileUpdate, job);
}

void CloudClient::updateScreen(Screen& screen)
{
    HttpJob job;

    tao::json::value root;
    root["id"] = screen.id();
    root["name"] = screen.name();
    root["ipList"] = screen.ipList();
    root["version"] = screen.version();

    job.target = "/screen/update";
    job.method = http::verb::post;
    job.context = tao::json::to_string(root);

    addHttpJob(JobCategories::ScreenUpdate, job);
}

void CloudClient::updateScreenStatus(Screen &screen)
{
    HttpJob job;

    tao::json::value root;
    root["id"] = screen.id();
    root["name"] = screen.name();
    root["status"] = screenStatusToString(screen.status());
    root["version"] = screen.version();

    job.target = "/screen/status/update";
    job.method = http::verb::post;
    job.context = tao::json::to_string(root);

    addHttpJob(JobCategories::ScreenUpdate, job);
}

void CloudClient::updateScreenError(Screen &screen)
{
    // TODO: implement this
}

void
CloudClient::reconnectWebsocket()
{
    m_websocket.reconnect(true);
}

void
CloudClient::shutdownWebsocket()
{
    m_websocket.shutdown();
}

bool
CloudClient::isWebsocketConnected() const
{
    return m_websocket.isConnected();
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
