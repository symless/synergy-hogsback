#ifndef WAMPCLIENT_H
#define WAMPCLIENT_H

#include "WampUtility.h"

#include <synergy/common/Logs.h>
#include <autobahn/autobahn.hpp>
// TODO: Figure out why AsioExecutor has to be included after autobahn
#include <synergy/common/AsioExecutor.h>
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>
#include <string>
#include <memory>
#include <stdexcept>
#include <boost/callable_traits.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/signals2.hpp>

namespace {

const int kKeepAliveIntervalSec = 5;

template <typename R>
struct WampCallHelper {
    template <typename Result> static
    void
    getReturnValue (Result&& result) {
        return result.template argument<R>(0);
    }
};

template <>
struct WampCallHelper<void> {
    template <typename Result> static
    void
    getReturnValue (Result&&) {
    }
};

template <typename Handler>
class WampEventHandler final {
    public:
        WampEventHandler (Handler&& handler): m_handler(std::move(handler)) {}
        WampEventHandler (Handler const& handler): m_handler(handler) {}
        void operator()(autobahn::wamp_event const& event);
    private:
        Handler m_handler;
};

template <typename Handler>
void
WampEventHandler<Handler>::operator()(autobahn::wamp_event const& event) {
    typename boost::fusion::result_of::invoke
                <make_tuple, boost::callable_traits::args_t<Handler>>::type args;
    event.get_arguments (args);
    boost::fusion::invoke (m_handler, args);
}

} // namespace

class WampClient
{
public:
    explicit WampClient (boost::asio::io_service& io);
    void start (std::string const& ip, int port);

    boost::asio::io_service&
    ioService() noexcept {
        return m_executor.underlying_executor().get_io_service();
    }

    bool isConnected() const;

    template <typename Result, typename... Args>
    boost::future<Result>
    call (char const* const fun, Args&&... args) {
        /* Asio requires that handlers be copyable, but packaged_task isn't,
         * so we have to allocate
         */
        auto task = std::make_shared<boost::packaged_task<Result()>> (
            [this, fun, args_tup = std::make_tuple (std::forward<Args>(args)...)]() mutable {
                return m_session->call (fun, std::move(args_tup), m_defaultCallOptions).then
                    (m_executor, [&, fun](boost::future<autobahn::wamp_call_result> result) {
                        try {
                            return WampCallHelper<Result>::getReturnValue (result.get());
                        }
                        catch (...) {
                            if (std::string(fun) == std::string(kKeepAliveFunction)) {
                                commonLog()->debug("rpc keep alive failed");
                            }
                            else {
                                commonLog()->error("rpc function call failed: {}", std::string(fun));
                            }
                            connectionError();
                        }
                    }
                );
            }
        );

        ioService().post(
            boost::bind(&boost::packaged_task<Result()>::operator(), task)
        );

        /* Actually returns a future<future<Result>, which means we depend on a
         * Boost extension which unwraps that in to a future<Result>
         */
        return task->get_future();
    }

    /* TODO: return the subscription object so we can easily unsubscribe
     */
    template <typename Handler>
    void
    subscribe (char const* const topic, Handler&& handler) {
        using handler_type = std::decay_t<Handler>;
        ioService().post([this, topic,
                         eventHandler = WampEventHandler<handler_type>
                            (std::forward<Handler>(handler))]() mutable {
            m_session->subscribe (topic, std::move(eventHandler));
        });
    }

    boost::signals2::signal<void()> connected;
    boost::signals2::signal<void()> connecting;
    boost::signals2::signal<void()> connectionError;

private:
    void connect();
    void keepAlive();

private:
    boost::executors::executor_adaptor<AsioExecutor> m_executor;
    std::shared_ptr<autobahn::wamp_session> m_session;
    std::shared_ptr<autobahn::wamp_transport> m_transport;
    autobahn::wamp_call_options m_defaultCallOptions;
    boost::asio::deadline_timer m_keepAliveTimer;
    bool m_connected;
};


#endif // WAMPCLIENT_H
