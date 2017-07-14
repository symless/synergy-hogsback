#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include <synergy/common/WampClient.h>
#include <tuple>
#include <type_traits>

class RpcClient {
public:
    template <typename... Args>
    using Future = WampClient::Future<Args...>;

    template <typename... Args>
    RpcClient (Args&&... args): m_wampClient(std::forward<Args>(args)...) {}

    RpcClient (RpcClient const&) = delete;
    RpcClient& operator= (RpcClient const&) = delete;

    boost::asio::io_service&
    getIoService() const noexcept {
        return m_wampClient.getIoService();
    }

    template <typename... Args>
    void start (Args&&... args) {
        return m_wampClient.start (std::forward<Args>(args)...);
    }

    template <typename R, typename... Args>
    Future<R> call (char const* const func, Args&&... args) {
        std::tuple<std::decay_t<Args>...> targs (std::forward<Args>(args)...);
        return m_wampClient.call<R> (func, std::move (targs));
    }

private:
    WampClient m_wampClient;
};

#endif // RPCCLIENT_H
