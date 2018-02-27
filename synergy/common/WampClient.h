#ifndef WAMPCLIENT_H
#define WAMPCLIENT_H

#include <synergy/common/WampUtility.h>
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
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

namespace {

template <typename Result>
struct WampCallHelper {
    template <typename Arg> static
    void
    complete (boost::promise<Result>& promise, Arg&& result) {
        auto value = result.template argument<Result>(0);
        promise.set_value(value);
    }
};

template <>
struct WampCallHelper<void> {
    template <typename Result> static
    void
    complete (boost::promise<void>& promise, Result&&) {
        promise.set_value();
    }
};

template <typename Handler>
class WampEventHandler final {
    public:
        WampEventHandler (Handler&& handler): m_handler (std::move(handler)) {}
        WampEventHandler (Handler const& handler): m_handler (handler) {}
        void operator()(autobahn::wamp_event const& event);
    private:
        Handler m_handler;
};

template <typename Handler> inline
void
WampEventHandler<Handler>::operator()
(autobahn::wamp_event const& event) {
    typename boost::fusion::result_of::invoke
                <make_tuple, boost::callable_traits::args_t<Handler>>::type
                    args;
    event.get_arguments (args);
    boost::fusion::invoke (m_handler, args);
}

} // namespace

class WampClient
{
public:
    WampClient (boost::asio::io_service& io,
                std::shared_ptr<spdlog::logger> logger);

    void
    connect (std::string const& ip, int port);

    void
    disconnect();

    auto&
    executor() noexcept {
        return m_executor;
    }

    auto&
    ioService() noexcept {
        return m_executor.underlying_executor().get_io_service();
    }

    template <typename Result, typename... Args>
    boost::future<Result>
    call (char const* const fun, Args&&... args) {
        auto promise = std::make_shared<boost::promise<Result>>();
        auto future = promise->get_future();

        ioService().dispatch ([
            this,
            fun,
            argsTup = std::make_tuple (std::forward<Args>(args)...),
            outerPromise = std::move (promise)
        ]() mutable {
            this->m_session->call (fun, std::move (argsTup),
                                   m_defaultCallOptions)
            .then (this->m_executor, [
                this,
                fun,
                promise = std::move (outerPromise)
            ](boost::future<autobahn::wamp_call_result> result) mutable {
                try {
                    WampCallHelper<Result>::complete (*promise, result.get());
                } catch (...) {
                    promise->set_exception (std::current_exception());
                    this->log()->error ("RPC call failed: {}", std::string(fun));
                    this->disconnected(false);
                }
            });
        });

        return future;
    }

    template <typename Handler>
    void
    subscribe (char const* const topic, Handler&& handler) {
        using handler_type = std::decay_t<Handler>;
        ioService().dispatch ([this, topic,
                               eventHandler = WampEventHandler<handler_type>
                                (std::forward<Handler>(handler))]() mutable {
            m_session->subscribe (topic, std::move(eventHandler));
        });
    }

    bool isConnected() const;
    boost::signals2::signal<void()> connected;
    boost::signals2::signal<void()> connecting;
    boost::signals2::signal<void(bool)> disconnected;

private:
    auto
    log() const {
        if (!m_logger) {
            std::array<spdlog::sink_ptr, 1> sinks {std::make_shared<spdlog::sinks::null_sink_mt>()};
            auto logger = std::make_shared<spdlog::logger>("null", begin(sinks), end(sinks));
            return logger;
        }
        return m_logger;
    }

    void connect();
    void keepAlive();

private:
    boost::executors::executor_adaptor<AsioExecutor> m_executor;
    std::shared_ptr<autobahn::wamp_transport> m_transport;
    std::shared_ptr<autobahn::wamp_session> m_session;
    autobahn::wamp_call_options m_defaultCallOptions;
    boost::asio::deadline_timer m_keepAliveTimer;
    std::shared_ptr<spdlog::logger> m_logger;
    bool m_connected = false;
};


#endif // WAMPCLIENT_H
