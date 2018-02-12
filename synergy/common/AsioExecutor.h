#ifndef SYNERGY_COMMON_ASIO_EXECUTOR_HPP
#define SYNERGY_COMMON_ASIO_EXECUTOR_HPP

#include <boost/asio/io_service.hpp>

class AsioExecutor final {
public:
    explicit AsioExecutor (boost::asio::io_service& io) noexcept:
        m_ioService(io) {
    }

    auto&
    get_io_service() const noexcept {
        return m_ioService;
    }

    bool
    try_executing_one() {
        return m_ioService.poll_one();
    }

    template <typename Work>
    void
    submit (Work&& work) {
        if (closed()) {
            return;
        }
        m_ioService.dispatch(std::forward<Work>(work));
    }

    void
    close() {
        /* Pretty sure this should never be called */
        m_ioService.stop();
    }

    bool
    closed() const noexcept {
        return m_ioService.stopped();
    }

    template <typename Predicate>
    bool
    reschedule_until (Predicate pred) {
        bool doneWork = false;
        do {
            if (m_ioService.run_one() != 0) {
                doneWork = true;
            }
        } while (!pred() && !closed());
        return doneWork;
    }

private:
    boost::asio::io_service& m_ioService;
};

#endif
