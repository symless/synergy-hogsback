#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include <synergy/common/WampClient.h>
#include <tuple>
#include <type_traits>

class RpcClient {
public:
    RpcClient (RpcClient const&) = delete;
    RpcClient& operator= (RpcClient const&) = delete;

    template <typename... Args>
    RpcClient (Args&&... args): m_wampClient(std::forward<Args>(args)...) {}

    boost::asio::io_service&
    get_io_service() noexcept {
        return m_wampClient.get_io_service();
    }

    template <typename... Args>
    decltype(auto)
    start (Args&&... args) {
        return m_wampClient.start (std::forward<Args>(args)...);
    }

    template <typename R, typename... Args>
    decltype(auto)
    call (char const* const fun, Args&&... args) {
        std::tuple<std::decay_t<Args>...> argsVal (std::forward<Args>(args)...);
        return m_wampClient.call<R> (fun, std::move (argsVal));
    }

private:
    WampClient m_wampClient;
};

#endif // RPCCLIENT_H
