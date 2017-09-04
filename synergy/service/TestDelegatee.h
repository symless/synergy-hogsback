#ifndef TESTDELEGATEE_H
#define TESTDELEGATEE_H

#include <list>
#include <string>
#include <boost/signals2.hpp>

class TestDelegatee
{
public:
    TestDelegatee(const std::list<std::string>& testCases, int batchSize);

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(std::map<std::string, bool>)> done;

    void start();
};

#endif // TESTDELEGATEE_H
