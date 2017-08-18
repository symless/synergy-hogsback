#include "CloudClient.h"

static const long kReconnectDelaySec = 3;

CloudClient::CloudClient(boost::asio::io_service& ioService) :
    m_ioService(ioService),
    m_httpSession(ioService),
    m_websocket(ioService)
{
    init();
}

void CloudClient::init()
{
    m_websocket.reconnectRequired.connect ([this]() {
        m_websocket.reconnect(kReconnectDelaySec);
    },
    boost::signals2::at_front);
    m_websocket.connect();
}
