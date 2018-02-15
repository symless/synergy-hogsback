#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>

class TrayService {
public:
    explicit
    TrayService (boost::asio::io_service& ioService):
        m_pingTimer(ioService) {
    }

    bool start();
    void stop();
    void ping();

private:
    bool m_trayRunning = false;
    boost::asio::steady_timer m_pingTimer;
};
