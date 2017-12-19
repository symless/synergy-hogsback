#include <synergy/service/SessionMonitor.h>

struct SessionMonitor::Impl {
};

SessionMonitor::SessionMonitor
(boost::asio::io_service& ioService): ioService_(ioService)
{}

SessionMonitor::~SessionMonitor()
{}

void SessionMonitor::start()
{}

void SessionMonitor::stop()
{}

void SessionMonitor::poll()
{}

std::vector<std::string>
SessionMonitor::sessions()
{
    return std::vector<std::string>();
}
