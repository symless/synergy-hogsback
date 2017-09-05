#include "TestDelegatee.h"


TestDelegatee::TestDelegatee(boost::asio::io_service &io, std::vector<std::string> &ipList, int batchSize) :
    m_ioService(io),
    m_ipList(std::move(ipList)),
    m_batchSize(batchSize)
{

}

TestDelegatee::~TestDelegatee()
{

}

void TestDelegatee::start()
{
    std::map<std::string, bool> results;

    // TODO: start the actual test

    done(results, m_batchSize);
}
