#include "ConnectivityTester.h"
#include "TestDelegatee.h"

#include <boost/algorithm/string.hpp>
#include <algorithm>

ConnectivityTester::ConnectivityTester(boost::asio::io_service &io) :
    m_testDelegatee(nullptr),
    m_ioService(io)
{

}

void ConnectivityTester::testNewScreens(const std::vector<ProfileSnapshot::Screen> &screens)
{
    std::set<int> latestScreenIdSet;
    int testCaseBatchSize = 0;
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
                testCaseBatchSize++;

                // add into set
                m_screenIdSet.insert(screenId);
            }
        }
    }

    // remove inactive screen id
    std::vector<int> result;
    std::set_intersection(
        latestScreenIdSet.begin(), latestScreenIdSet.end(),
        m_screenIdSet.begin(), m_screenIdSet.end(),
        std::back_inserter(result));
    m_screenIdSet = std::move(std::set<int>(result.begin(), result.end()));

    // start connectivity test
    if (testCaseBatchSize > 0 && !m_testDelegatee) {
        startTesting(testCaseBatchSize);
    }
}

std::vector<std::string> ConnectivityTester::getSuccessfulResults(int screenId) const
{
    auto it = m_screenSuccessfulResults.find(screenId);
    if (it != m_screenSuccessfulResults.end()) {
        return it->second;
    }
}

void ConnectivityTester::startTesting(int batchSize)
{
    std::vector<std::string> testIpList;
    int i = 0;
    for (auto testCase : m_pendingTestCases) {
        std::vector<std::string> ipList = extractIpListFromTestCase(testCase);
        testIpList.insert(testIpList.begin(), ipList.begin(), ipList.end()) ;

        i++;
        if (i >= batchSize) {
            break;
        }
    }

    m_testDelegatee = new TestDelegatee(m_ioService, std::move(testIpList), batchSize);
    m_testDelegatee->done.connect(std::bind(&ConnectivityTester::onTestDelegateeDone, this, std::placeholders::_1, std::placeholders::_2));
    m_testDelegatee->start();
}

void ConnectivityTester::onTestDelegateeDone(std::map<std::string, bool> results, int batchSize)
{
    // iterate through all tested cases
    int testedCount = 0;
    for (auto testCase : m_pendingTestCases) {
        std::vector<std::string> ipList = extractIpListFromTestCase(testCase);

        std::vector<std::string> successfulIpList;
        std::vector<std::string> failedIpList;

        // extract successful and failed test results
        for (auto ip : ipList) {
            bool r = results[ip];

            if (r) {
                successfulIpList.emplace_back(ip);
            }
            else {
                failedIpList.emplace_back(ip);
            }
        }

        if (boost::optional<int> screenId = extractScreenIdFromTestCase(testCase)) {
            std::string successfulIp = boost::algorithm::join(successfulIpList, ",");
            std::string failedIp = boost::algorithm::join(failedIpList, ",");

            // TODO: report to cloud

            // update connectivity results
            m_screenSuccessfulResults[*screenId] = successfulIpList;
        }

        testedCount++;
        if (testedCount >= batchSize) {
            break;
        }
    }

    for (int i = 0; i < batchSize; i++) {
        m_pendingTestCases.pop_front();
    }

    delete m_testDelegatee;
    m_testDelegatee = nullptr;

    if (!m_pendingTestCases.empty()) {
        startTesting(m_pendingTestCases.size());
    }
}

std::vector<std::string> ConnectivityTester::extractIpListFromTestCase(std::string testCase)
{
    std::vector<std::string> ipList;

    std::vector<std::string> parts;
    boost::split(parts, testCase, boost::is_any_of(","));

    if (parts.size() > 1) {
        for (int i = 1; i < parts.size(); i++) {
            ipList.emplace_back(std::move(parts[i]));
        }
    }

    return std::move(ipList);
}

boost::optional<int> ConnectivityTester::extractScreenIdFromTestCase(std::string testCase)
{
    std::vector<std::string> parts;
    boost::split(parts, testCase, boost::is_any_of(","));

    if (parts.size() >= 1) {
        return std::stoi(parts[0]);
    }

    return boost::none;
}
