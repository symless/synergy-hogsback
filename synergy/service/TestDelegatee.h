#ifndef TESTDELEGATEE_H
#define TESTDELEGATEE_H

#include <list>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
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
    void onTestFinish();

private:
    boost::asio::io_service& m_ioService;
    boost::asio::deadline_timer m_timeout;
    std::vector<std::string> m_ipList;
    int m_batchSize;
    std::map<std::string, bool> m_results;
};

#endif // TESTDELEGATEE_H
