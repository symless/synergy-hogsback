#ifndef SERVICEWORKER_H
#define SERVICEWORKER_H

#include "synergy/service/IOService.h"

class ServiceWorker
{
public:
    ServiceWorker(boost::asio::io_service& threadIoService);

    void start();

private:
    boost::asio::io_service& m_threadIoService;
};

#endif // SERVICEWORKER_H
