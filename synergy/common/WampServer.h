#ifndef WAMPSERVER_H
#define WAMPSERVER_H

#include <synergy/common/WampUtility.h>
#include <synergy/common/AsioExecutor.h>

#include <memory>
#include <functional>
#include <type_traits>
#include <autobahn/autobahn.hpp>
#include <boost/signals2.hpp>
#include <boost/callable_traits.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>

namespace {

template <typename Result>
struct WampCalleeInvoker final {
    template <typename Fun, typename Args>
    void
    invoke (autobahn::wamp_invocation& invocation, Fun&& fun, Args&& args) {
        invocation->result (std::make_tuple
                            (boost::fusion::invoke (std::forward<Fun>(fun),
                                                   std::forward<Args>(args))));
    }
};

template <>
struct WampCalleeInvoker<void> final {
    template <typename Fun, typename Args>
    void
    invoke (autobahn::wamp_invocation& invocation, Fun&& fun, Args&& args) {
        boost::fusion::invoke (std::forward<Fun>(fun), std::forward<Args>(args));
        invocation->empty_result();
    }
};

template <typename Fun>
class WampCallee final {
    public:
        WampCallee (Fun&& fun): m_fun (std::move(fun)) {}
        WampCallee (Fun const& fun): m_fun (fun) {}
        void operator()(autobahn::wamp_invocation);

    private:
        Fun m_fun;
};

template <typename Fun> inline
void
WampCallee<Fun>::operator()(autobahn::wamp_invocation invocation) {
    /* Create a tuple of values based on the arguments that need to be passed
     * to the function. callable_traits maintains references and qualifiers, so
     * a tuple of these arguments can't be constructed directly. Instead we
     * apply the transformation that would occur if we had called
     * std::make_tuple() with those arguments, and create a new instance of the
     * resulting tuple type.
     */
    typename boost::fusion::result_of::invoke
                <make_tuple, boost::callable_traits::args_t<Fun>>::type args;

    /* Create an invoker object which deals with the return value / void
     * appropriately wrt to the invocation handle object.
     */
    using return_type = boost::callable_traits::return_type_t<Fun>;
    WampCalleeInvoker<return_type> invoker;
    invoker.invoke (invocation, m_fun, args);
}

} // namespace

class WampServer final {
public:
    explicit WampServer (boost::asio::io_service& ioService);
    WampServer (WampServer const&) = delete;
    WampServer& operator= (WampServer const&) = delete;

    void
    start (std::string const& ip, int port);

    auto&
    executor() noexcept {
        return m_executor;
    }

    boost::asio::io_service&
    ioService() noexcept {
        return executor().underlying_executor().get_io_service();
    }

    template <typename Fun>
    void
    provide (char const* const name, Fun&& fun) {
        using fun_type = std::decay_t<Fun>;
        ioService().dispatch ([
            this,
            name,
            callee = WampCallee<fun_type>(std::forward<Fun>(fun))
        ]() mutable {
            m_session->provide (name, std::move (callee));
        });
    }

    template <typename... Args>
    void
    publish (char const* const topic, Args&&... args) {
        ioService().dispatch ([
            this,
            topic,
            argsTup = std::make_tuple(std::forward<Args>(args)...)
        ]() mutable {
            m_session->publish (topic, std::move (argsTup));
        });
    }

public:
    boost::signals2::signal<void()> ready;

private:
    boost::executors::executor_adaptor<AsioExecutor> m_executor;
    std::shared_ptr<autobahn::wamp_session> m_session;
    std::shared_ptr<autobahn::wamp_transport> m_transport;
};

#endif // WAMPSERVER_H
