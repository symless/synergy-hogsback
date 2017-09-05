#include "TestDelegatee.h"

static const long kTimeoutSec = 3;

TestDelegatee::TestDelegatee(boost::asio::io_service &io, std::vector<std::string> &ipList, int batchSize) :
    m_ioService(io),
    m_timeout(io),
    m_ipList(std::move(ipList)),
    m_batchSize(batchSize)
{

}

TestDelegatee::~TestDelegatee()
{

}

void TestDelegatee::start()
{
    for (auto ip : m_ipList) {
        // TODO: start the test in parallel
    }

    m_timeout.cancel();
    m_timeout.expires_from_now(boost::posix_time::seconds(kTimeoutSec));

    m_timeout.async_wait([this](const boost::system::error_code&) {
        onTestFinish();
    });
}

void TestDelegatee::onTestFinish()
{
    done(m_results, m_batchSize);
}
