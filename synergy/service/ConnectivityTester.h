#ifndef CONNECTIVITYTESTER_H
#define CONNECTIVITYTESTER_H

#include "ProfileSnapshot.h"

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <set>
#include <vector>
#include <string>

class TestDelegatee;

class ConnectivityTester
{
public:
    ConnectivityTester(boost::asio::io_service &io);

    void testNewScreens(const std::vector<ProfileSnapshot::Screen>& screens);

    //std::vector<std::string> getSuccessfulResults(int screenId) const;

//    using signal = boost::signals2::signal<Args...>;

//private slots:

//    void onNewConnection();
//    void onConnectionReadyRead();
//
//
private:
    void startTesting();
    void onTestDelegateeDone(std::map<std::string, bool> results);

private:
    std::set<int> m_screenIdSet;
    std::list<std::string> m_pendingTestCases;
    std::string m_localHostname;
    TestDelegatee* m_testDelegatee;
    int m_testCaseBatchSize;
    boost::asio::io_service& m_ioService;
//    QTcpServer* m_tcpServer;
//    QMap<int, QStringList> m_screenSuccessfulResults;
};

#endif // CONNECTIVITYTESTER_H
