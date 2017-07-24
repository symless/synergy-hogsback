#ifndef RPCSERVER_H
#define RPCSERVER_H

#include <memory>
#include <functional>
#include <type_traits>
#include <autobahn/autobahn.hpp>
#include <boost/signals2.hpp>
#include <boost/callable_traits.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/asio.hpp>
#include <synergy/common/AsioExecutor.h>

namespace {

template <typename Fun>
class WampCallee final {
    public:
        WampCallee (Fun&& fun): impl_(std::move(fun)) {}
        WampCallee (Fun const& fun): impl_(fun) {}
        void operator()(autobahn::wamp_invocation);
    private:
        Fun impl_;
};

template <typename Fun> inline
void
WampCallee<Fun>::operator()(autobahn::wamp_invocation invocation) {
    // Note: this doesn't work if the callable takes arguments by reference
    // TODO: apply std::decay_t as a transformation
    boost::callable_traits::args_t<Fun> args;
    invocation->get_arguments (args);
    boost::fusion::invoke (impl_, args);
}

} // namespace


class RpcServer final {
public:
    RpcServer (boost::asio::io_service& io, std::string ipAddress, int port);
    RpcServer (RpcServer const&) = delete;
    RpcServer& operator= (RpcServer const&) = delete;
    void start();

    template <typename Fun> inline
    decltype(auto)
    provide (char const* const name, Fun&& fun) {
        using fun_type = std::decay_t<Fun>;
        return m_session->provide (name,
                                   WampCallee<fun_type>(std::forward<Fun>(fun)));
    }

public:
    boost::signals2::signal<void()> ready;

private:
    boost::executors::executor_adaptor<AsioExecutor> m_executor;
    std::shared_ptr<autobahn::wamp_session> m_session;
    std::shared_ptr<autobahn::wamp_transport> m_transport;
};

#endif // RPCSERVER_H
