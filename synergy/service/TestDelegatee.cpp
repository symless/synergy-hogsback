#include "TestDelegatee.h"

TestDelegatee::TestDelegatee(const std::list<std::string> &testCases, int batchSize)
{

}

void TestDelegatee::start()
{
    std::map<std::string, bool> results;

    // TODO: start the actual test

    done(results);
}
