#include "TestDelegatee.h"

TestDelegatee::TestDelegatee(boost::asio::io_service &io, const std::list<std::string> &testCases, int batchSize) :
    m_ioService(io)
{

}

void TestDelegatee::start()
{
    std::map<std::string, bool> results;

    // TODO: start the actual test

    done(results);
}
