#include "ConnectivityTester.h"
#include "TestDelegatee.h"

#include <algorithm>

ConnectivityTester::ConnectivityTester(boost::asio::io_service &io) :
    m_testDelegatee(nullptr),
    m_ioService(io)
{

}

void ConnectivityTester::testNewScreens(const std::vector<ProfileSnapshot::Screen> &screens)
{
    std::set<int> latestScreenIdSet;
    for (ProfileSnapshot::Screen const& screen : screens) {
        // skip inactive screens
        if (screen.active == "false") {
            continue;
        }

        std::string screenName = screen.name;

        // skip local screen
        if (screenName != m_localHostname) {
            int screenId = screen.id;
            latestScreenIdSet.insert(screenId);
            std::set<int>::const_iterator i = m_screenIdSet.find(screenId);
            // if this is a new screen and there is not a connectivity test running already
            if (i == m_screenIdSet.end() && !m_testDelegatee) {
                // get ip list
                std::string ipList = screen.ipList;
                if (ipList.empty()) {
                    continue;
                }
                // combine screen id and ip list separated by comma
                std::string testCase = std::to_string(screenId);
                testCase += ',';
                testCase += ipList;

                // add connectivity test case
                m_pendingTestCases.emplace_back(std::move(testCase));
                m_testCaseBatchSize++;

                // add into set
                m_screenIdSet.insert(screenId);
            }
        }


        // start connectivity test
        if (m_testCaseBatchSize > 0 && !m_testDelegatee) {
            startTesting();
        }

        // remove inactive screen id
        std::vector<int> result;
        std::set_intersection(
            latestScreenIdSet.begin(), latestScreenIdSet.end(),
            m_screenIdSet.begin(), m_screenIdSet.end(),
            std::back_inserter(result));
        m_screenIdSet = std::move(std::set<int>(result.begin(), result.end()));
    }
}

void ConnectivityTester::startTesting()
{
    m_testDelegatee = new TestDelegatee(m_ioService, m_pendingTestCases, m_testCaseBatchSize);
    m_testDelegatee->done.connect(std::bind(&ConnectivityTester::onTestDelegateeDone, this, std::placeholders::_1));
    m_testDelegatee->start();
}

void ConnectivityTester::onTestDelegateeDone(std::map<std::string, bool> results)
{
    // TODO: process the test results

    delete m_testDelegatee;
    m_testDelegatee = nullptr;

    // TODO: check if there is some pending test left
}
