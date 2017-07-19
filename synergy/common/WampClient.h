#ifndef WAMPCLIENT_H
#define WAMPCLIENT_H

#include <boost/asio.hpp>
#include <autobahn/autobahn.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/executor.hpp>
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


class AsioExecutor final {
public:
    explicit AsioExecutor (boost::asio::io_service& io) noexcept:
        m_io_service(io) {}
    auto& get_io_service() const noexcept { return m_io_service; }

    bool
    try_executing_one() {
        return m_io_service.poll_one();
    }

    template <typename Work>
    void
    submit (Work&& work) {
        if (closed()) {
            throw std::runtime_error ("AsioExecutor has been closed");
        }
        /* Note: this allows work to run on the calling thread */
        m_io_service.dispatch (std::forward<Work>(work));
    }

    void
    close() {
        /* Pretty sure this should never be called */
        m_io_service.stop();
    }

    bool
    closed() const noexcept {
        return m_io_service.stopped();
    }

    template <typename Predicate>
    bool
    reschedule_until (Predicate pred) {
        bool doneWork = false;
        do {
            if (m_io_service.run_one() != 0) {
                doneWork = true;
            }
        } while (!pred() && !closed());
        return doneWork;
    }

private:
    boost::asio::io_service& m_io_service;
};


class WampClient
{
public:
    template <typename... Args>
    using future_t = boost::future<Args...>;

    explicit WampClient (boost::asio::io_service& io);

    boost::asio::io_service&
    get_io_service ()
    noexcept {
        return m_executor.underlying_executor().get_io_service();
    }

    future_t<void>
    start (std::string ip, int port, bool debug = true);

    template <typename Result, typename Args>
    decltype(auto)
    call (char const* const fun, Args&& args) {
        return m_session->call (fun, std::forward<Args>(args),
                                m_default_call_options).then
            (m_executor, [](boost::future<autobahn::wamp_call_result> result) {
                return WampCallHelper<Result>::get_return_value (result.get());
            }
        );
    }

private:
    boost::executors::executor_adaptor<AsioExecutor> m_executor;
    std::shared_ptr<autobahn::wamp_session> m_session;
    autobahn::wamp_call_options m_default_call_options;
};


#endif // WAMPCLIENT_H
