#ifndef TESTDELEGATEE_H
#define TESTDELEGATEE_H

#include <list>
#include <string>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>

class TestDelegatee
{
public:
    TestDelegatee(boost::asio::io_service &io, std::vector<std::string> &ipList, int batchSize);
    ~TestDelegatee();

    TestDelegatee(const TestDelegatee&) = delete;
    TestDelegatee(const TestDelegatee&&) = delete;
    TestDelegatee& operator= (const TestDelegatee&) = delete;
    TestDelegatee& operator= (const TestDelegatee&&) = delete;

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(std::map<std::string, bool>, int)> done;

    void start();

private:
    boost::asio::io_service& m_ioService;
    std::vector<std::string> m_ipList;
    int m_batchSize;
};

#endif // TESTDELEGATEE_H
