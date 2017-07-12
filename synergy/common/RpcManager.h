#ifndef RPCMANAGER_H
#define RPCMANAGER_H

#include <boost/asio.hpp>
#include <thread>

class WampServer;
class WampRouter;

class RpcManager
{
public:
    RpcManager(boost::asio::io_service& ioService);
    ~RpcManager();

    void initRouterAndServer();
    void initClient();
    void startRouter();

    auto getServer() { return m_server; }

private:
    boost::asio::io_service& m_mainIoService;
    std::shared_ptr<WampServer> m_server;
    std::shared_ptr<WampRouter> m_router;
    std::thread m_routerThread;

};

#endif // RPCMANAGER_H
