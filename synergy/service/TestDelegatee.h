#ifndef TESTDELEGATEE_H
#define TESTDELEGATEE_H

#include <list>
#include <string>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>

class TestDelegatee
{
public:
    TestDelegatee(boost::asio::io_service &io, const std::list<std::string>& testCases, int batchSize);

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(std::map<std::string, bool>)> done;

    void start();

private:
    boost::asio::io_service& m_ioService;
};

#endif // TESTDELEGATEE_H
