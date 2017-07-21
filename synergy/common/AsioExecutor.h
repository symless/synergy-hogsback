#ifndef SYNERGY_COMMON_ASIO_EXECUTOR_HPP
#define SYNERGY_COMMON_ASIO_EXECUTOR_HPP

#include <boost/asio.hpp>

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
            throw std::runtime_error ("I/O executor has been closed");
        }
        /* Note: this allows work to run on the calling thread. */
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

#endif
