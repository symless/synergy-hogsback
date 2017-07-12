#ifndef WAMPCLIENT_H
#define WAMPCLIENT_H

#include <boost/asio.hpp>
#include <string>

namespace autobahn
{
class wamp_session;
}

class WampClient
{
public:
    WampClient(boost::asio::io_service& ioService);
    void run(std::string ip, int port, bool debug = true);
    void startCore(std::vector<std::string> cmd);
private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<boost::asio::io_service::work> m_work;
    std::shared_ptr<autobahn::wamp_session> m_session;
};

#endif // WAMPCLIENT_H
