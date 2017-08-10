#ifndef RPCMANAGER_H
#define RPCMANAGER_H

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

class WampServer;
class WampRouter;

class RpcManager
{
public:
    RpcManager(boost::asio::io_service& ioService);
    ~RpcManager();

    void start();
    void stop();

    auto router() const noexcept {
        return m_router;
    }

    auto server() const noexcept {
        return m_server;
    }

    const char* ipAddress() const;
    const int port() const;

public:
    boost::signals2::signal<void()> ready;

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<WampRouter> m_router;
    std::shared_ptr<WampServer> m_server;
};

#endif // RPCMANAGER_H
