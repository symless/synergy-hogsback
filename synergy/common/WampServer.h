#ifndef WAMPSERVER_H
#define WAMPSERVER_H

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

struct make_tuple {
    template <typename... Args>
    auto operator()(Args&&... args) const noexcept {
        return std::make_tuple (std::forward<Args>(args)...);
    }
};

template <typename Fun>
class WampCallee final {
    public:
        WampCallee (Fun&& fun): m_fun(std::move(fun)) {}
        WampCallee (Fun const& fun): m_fun(fun) {}
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
     * std::make_tuple with those arguments and create a new instance of the
     * resulting tuple type.
     */
    typename boost::fusion::result_of::invoke
                <make_tuple, boost::callable_traits::args_t<Fun>>::type args;

    invocation->get_arguments (args);
    boost::fusion::invoke (m_fun, args);
}

} // namespace


class WampServer final {
public:
    WampServer (boost::asio::io_service& io);
    WampServer (WampServer const&) = delete;
    WampServer& operator= (WampServer const&) = delete;

    boost::asio::io_service&
    ioService() /* const */ noexcept {
        return m_executor.underlying_executor().get_io_service();
    }

    void start (std::string const& ip, int port);

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

#endif // WAMPSERVER_H
