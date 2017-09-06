#include "TerminationSignalListener.h"
#include <iostream>

TerminationSignalListener::TerminationSignalListener
(boost::asio::io_service &io) :
#if defined(SIGQUIT)
    m_signals(io, SIGTERM, SIGINT, SIGQUIT)
#else
    m_signals(io, SIGTERM, SIGINT)
#endif
{
    m_signals.async_wait([this](auto&... args) {
        this->terminationHandler (args...);
    });
}

void
TerminationSignalListener::terminationHandler
(boost::system::error_code const& errorCode, int const signalNumber)
{
    if (errorCode == boost::asio::error::operation_aborted) {
        return;
    }
#if defined(SIGQUIT)
    if (signalNumber == SIGINT || signalNumber == SIGTERM || signalNumber == SIGQUIT) {
#else
    if (signalNumber == SIGINT || signalNumber == SIGTERM) {
#endif
        m_handler();
    }
}
