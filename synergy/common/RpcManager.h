#ifndef RPCMANAGER_H
#define RPCMANAGER_H

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

class WampServer;
class WampRouter;

class RpcManager final
{
public:
    explicit RpcManager(boost::asio::io_service& ioService,
                        std::string ip = "127.0.0.1", int port = 24888);
    ~RpcManager() noexcept;

    void start();
    void stop();
    std::shared_ptr<WampServer> server(); /* TODO: remove */
    std::string ip() const;
    int port() const;

public:
    boost::signals2::signal<void()> ready;

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<WampRouter> m_router;
    std::shared_ptr<WampServer> m_server;
    std::string m_ip;
    int m_port;
};

#endif // RPCMANAGER_H
