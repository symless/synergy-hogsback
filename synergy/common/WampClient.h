#ifndef WAMPCLIENT_H
#define WAMPCLIENT_H

#include <boost/asio.hpp>
#include <autobahn/autobahn.hpp>
#include <boost/thread/future.hpp>
#include <string>

class WampClient
{
public:
    template <typename... Args>
    using Future = boost::future<Args...>;

    explicit WampClient (boost::asio::io_service& ioService);

    boost::asio::io_service&
    getIoService () const noexcept {
        return m_ioService;
    }

    void start (std::string ip, int port, bool debug = true);

    template <typename R, typename Args>
    decltype(auto)
    call (char const* const func, Args&& args) {
        autobahn::wamp_call_options call_options;
        call_options.set_timeout (std::chrono::seconds(10));
        return m_session->call(func, std::forward<Args>(args), call_options).then
                ([](boost::future<autobahn::wamp_call_result> result) {
            result.get();
        });
    }

private:
    boost::asio::io_service& m_ioService;
    std::shared_ptr<autobahn::wamp_session> m_session;
};

#endif // WAMPCLIENT_H
