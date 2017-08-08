#include "TerminationSignalListener.h"

#include <iostream>

TerminationSignalListener::TerminationSignalListener(boost::asio::io_service &io) :
#if defined(SIGQUIT)
    m_signals(io, SIGTERM, SIGINT, SIGQUIT)
#else
    m_signals(io, SIGTERM, SIGINT)
#endif
{
    m_signals.async_wait(
                boost::bind(&TerminationSignalListener::terminationHandler, this, _1, _2));
}

void
TerminationSignalListener::terminationHandler(
    const boost::system::error_code& errorCode, int signalNumber)
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
