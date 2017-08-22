#include "CloudClient.h"

static const char* kPubSubServerHostname = "165.227.29.181";
static const char* kPubSubServerPort = "8080";
static const char* kCloudServerHostname = "192.168.3.93";
static const char* kCloudServerPort = "80";

CloudClient::CloudClient(boost::asio::io_service& ioService) :
    m_ioService(ioService),
    m_httpSession(ioService, kCloudServerHostname, kCloudServerPort),
    // TODO: get user ID as the unique pubsub channel
    m_websocket(ioService, kPubSubServerHostname, "/pubsub/auth/1", kPubSubServerPort)
{
    init();
}

void CloudClient::init()
{
    m_websocket.connect();
}
