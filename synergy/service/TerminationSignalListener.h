#ifndef TERMINATIONSIGNALLISTENER_H
#define TERMINATIONSIGNALLISTENER_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <functional>

class TerminationSignalListener
{
public:
    TerminationSignalListener(boost::asio::io_service& io);

    template<typename FuncT>
    void setHandler(FuncT&& handler) {
        m_handler = std::forward<FuncT>(handler);
    }

private:
    void terminationHandler(
            const boost::system::error_code& errorCode,
            int signalNumber);

private:
    boost::asio::signal_set m_signals;
    std::function<void ()> m_handler;
};

#endif // TERMINATIONSIGNALLISTENER_H
