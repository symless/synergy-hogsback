#ifndef WAMPCLIENT_H
#define WAMPCLIENT_H

#include <autobahn/autobahn.hpp>
#include <synergy/common/AsioExecutor.h>
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>
#include <string>
#include <stdexcept>

namespace {

template <typename R>
struct WampCallHelper {
    static
    decltype(auto)
    get_return_value (autobahn::wamp_call_result& result) {
        return result.argument<R>(0);
    }
};

template <>
struct WampCallHelper<void> {
    static
    void
    get_return_value (autobahn::wamp_call_result&) {
    }
};

} // namespace

class WampClient
{
public:
    template <typename... Args>
    using future_t = boost::future<Args...>;

    explicit WampClient (boost::asio::io_service& io);

    boost::asio::io_service&
    get_io_service () /* const */
    noexcept {
        return m_executor.underlying_executor().get_io_service();
    }

    future_t<void>
    start (std::string ip, int port, bool debug = true);

    template <typename Result, typename Args>
    decltype(auto)
    call (char const* const fun, Args&& args) {
        auto& io = get_io_service();
        io.post ([this, fun, args = std::forward<Args>(args)](){
            m_session->call (fun, std::move(args), m_default_call_options).then
                (m_executor, [](boost::future<autobahn::wamp_call_result> result) {
                    return WampCallHelper<Result>::get_return_value (result.get());
                }
            );
        });
    }

private:
    boost::executors::executor_adaptor<AsioExecutor> m_executor;
    std::shared_ptr<autobahn::wamp_session> m_session;
    autobahn::wamp_call_options m_default_call_options;
};


#endif // WAMPCLIENT_H
