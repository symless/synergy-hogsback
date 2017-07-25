#ifndef WAMPCLIENT_H
#define WAMPCLIENT_H

#include <autobahn/autobahn.hpp>
// NOTE: AsioExecutor has to be included after autobahn
#include <synergy/common/AsioExecutor.h>
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>
#include <string>
#include <memory>
#include <stdexcept>

namespace {

template <typename R>
struct WampCallHelper {
    template <typename Result> static
    void
    get_return_value (Result&& result) {
        return result.template argument<R>(0);
    }
};

template <>
struct WampCallHelper<void> {
    template <typename Result> static
    void
    get_return_value (Result&&) {
    }
};

} // namespace

class WampClient
{
public:
    explicit WampClient (boost::asio::io_service& io);

    boost::asio::io_service&
    ioService() /* const */ noexcept {
        return m_executor.underlying_executor().get_io_service();
    }

    boost::future<void>
    start (std::string const& ip, int port);

    template <typename Result, typename... Args>
    boost::future<Result>
    call (char const* const fun, Args&&... args) {
        /* Asio requires that handlers be copyable, but packaged_task isn't,
         * so we have to allocate */
        auto task = std::make_shared<boost::packaged_task<Result()>> (
            [this, fun, args = std::make_tuple (std::forward<Args>(args)...)]() mutable {
                return m_session->call (fun, std::move(args), m_default_call_options).then
                    (m_executor, [](boost::future<autobahn::wamp_call_result> result) {
                        return WampCallHelper<Result>::get_return_value (result.get());
                    }
                );
            }
        );

        ioService().post
            (boost::bind(&boost::packaged_task<Result()>::operator(), task));

        /* Returns a future<future<Result>, so we actually depend on a Boost
         * extension to unwrap that in to a future<Result> here */
        return task->get_future();
    }

private:
    boost::executors::executor_adaptor<AsioExecutor> m_executor;
    std::shared_ptr<autobahn::wamp_session> m_session;
    std::shared_ptr<autobahn::wamp_transport> m_transport;
    autobahn::wamp_call_options m_default_call_options;
};


#endif // WAMPCLIENT_H
