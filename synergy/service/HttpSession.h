#ifndef HTTPSESSION_H
#define HTTPSESSION_H

#include <boost/asio/io_service.hpp>

class HttpSession
{
public:
    HttpSession(boost::asio::io_service& ioService);
};

#endif // HTTPSESSION_H
