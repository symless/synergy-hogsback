#ifndef RPCMANAGER_H
#define RPCMANAGER_H

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

class WampServer;
class WampRouter;

class RpcManager final
{
public:
    explicit RpcManager(boost::asio::io_service& ioService);
    ~RpcManager() noexcept;

    void start();
    void stop();
    std::shared_ptr<WampServer> server(); /* TODO: remove */
    std::string ipAddress() const;
    int port() const;

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<WampRouter> m_router;
    std::shared_ptr<WampServer> m_server;

public:
    boost::signals2::signal<void()> ready;
};

#endif // RPCMANAGER_H
