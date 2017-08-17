#include "CloudClient.h"

CloudClient::CloudClient(boost::asio::io_service& ioService) :
    m_ioService(ioService),
    m_httpSession(ioService),
    m_websocket(ioService)
{
    init();
}

void CloudClient::init()
{
    m_websocket.connect();
}
