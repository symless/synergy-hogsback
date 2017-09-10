#ifndef CONNECTIVITYTESTER_H
#define CONNECTIVITYTESTER_H

#include <synergy/common/ProfileConfig.h>
#include <synergy/service/SecuredTcpServer.h>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <set>
#include <vector>
#include <string>

class TestDelegatee;

class ConnectivityTester final
{
public:
    ConnectivityTester(boost::asio::io_service &io, std::shared_ptr<ProfileConfig> localProfile);

    void testNewScreens(std::vector<Screen> addedScreens);
    std::vector<std::string> getSuccessfulResults(int screenId) const;

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void(int screenId, std::string successfulIp, std::string failedIp)> newReportGenerated;
    signal<void()> testBatchFinished;

    static const std::string testServerCertificate();
    static const std::string testServerKey();
    static const std::string testServerDH();

private:
    void startTestServer();
    void startTesting(int batchSize);
    void onTestDelegateeDone(std::map<std::string, bool> results, int batchSize);
    std::vector<std::string> extractIpListFromTestCase(std::string testCase);
    boost::optional<int> extractScreenIdFromTestCase(std::string testCase);

private:
    std::list<std::string> m_pendingTestCases;
    std::string m_localHostname;
    TestDelegatee* m_testDelegatee;
    boost::asio::io_service& m_ioService;
    std::unique_ptr<SecuredTcpServer> m_testServer;
    std::map<int, std::vector<std::string>> m_screenSuccessfulResults;
    std::shared_ptr<ProfileConfig> m_localProfileConfig;
};

#endif // CONNECTIVITYTESTER_H
