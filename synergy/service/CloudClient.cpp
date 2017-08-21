#include "CloudClient.h"

static const char* kPubSubServerHostname = "192.168.3.93";
static const char* kPubSubServerPort = "80";
static const char* kCloudServerHostname = "192.168.3.93";
static const char* kCloudServerPort = "80";

CloudClient::CloudClient(boost::asio::io_service& ioService) :
    m_ioService(ioService),
    m_httpSession(ioService, kCloudServerHostname, kCloudServerPort),
    m_websocket(ioService, kPubSubServerHostname, kPubSubServerPort)
{
    init();
}

void CloudClient::init()
{
    m_websocket.connect();
}
