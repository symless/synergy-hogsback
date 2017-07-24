#ifndef RPCMANAGER_H
#define RPCMANAGER_H

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

class RpcServer;
class WampRouter;

class RpcManager
{
public:
    RpcManager(boost::asio::io_service& ioService);
    ~RpcManager();

    void start();
    void stop();

    template <typename... Args>
    decltype (auto) provide (Args&&... args) {
        return m_server->provide(std::forward<Args>(args)...);
    }

public:
    boost::signals2::signal<void()> ready;

private:
    boost::asio::io_service& m_io;
    std::shared_ptr<WampRouter> m_router;
    std::shared_ptr<RpcServer> m_server;
};

#endif // RPCMANAGER_H
