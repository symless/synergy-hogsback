#ifndef WAMPROUTER_H
#define WAMPROUTER_H

#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>
#include <spdlog/logger.h>

namespace bonefish {
    class wamp_routers;
    class wamp_serializers;
    class rawsocket_server;
}

class WampRouter
{
public:
    explicit WampRouter (boost::asio::io_service& ioService,
                         std::shared_ptr<spdlog::logger> log);
    ~WampRouter ();

    void start (std::string const& ip, int port);
    void stop();

    auto&
    ioService() const {
        return m_ioService;
    }

    auto
    log() const {
        return m_log;
    }

    boost::signals2::signal<void()> ready;

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<spdlog::logger> m_log;
    std::shared_ptr<bonefish::wamp_routers> m_routers;
    std::shared_ptr<bonefish::wamp_serializers> m_serializers;
    std::shared_ptr<bonefish::rawsocket_server> m_rawsocketServer;
};

#endif // WAMPROUTER_H
