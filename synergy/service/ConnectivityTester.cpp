#include "ConnectivityTester.h"

#include <synergy/service/TestDelegatee.h>
#include <synergy/service/Logs.h>

#include <boost/algorithm/string.hpp>
#include <algorithm>

static const std::string kConnectivityTestIp = "0.0.0.0";
static const std::string kConnectivityTestPort = "24810";

ConnectivityTester::ConnectivityTester(boost::asio::io_service &io, std::shared_ptr<Profile> localProfile) :
    m_localHostname(boost::asio::ip::host_name()),
    m_testDelegatee(nullptr),
    m_ioService(io),
    m_localProfile(localProfile)
{
    startTestServer();

    m_localProfile->screenSetChanged.connect(
        [this](std::vector<Screen> added, std::vector<Screen> removed){
            testNewScreens(added);
        }
    );
}

void ConnectivityTester::testNewScreens(std::vector<Screen> addedScreens)
{
    int testCaseBatchSize = 0;
    for (auto const& screen : addedScreens) {
        // skip inactive screens
        if (!screen.active()) {
            continue;
        }

        // skip local screen
        if (screen.name() != m_localHostname) {
            // get ip list
            std::string ipList = screen.ipList();
            if (ipList.empty()) {
                continue;
            }
            // combine screen id and ip list separated by comma
            std::string testCase = std::to_string(screen.id());
            testCase += ',';
            testCase += ipList;

            // add connectivity test case
            m_pendingTestCases.emplace_back(std::move(testCase));
            testCaseBatchSize++;
        }
    }

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

const std::string ConnectivityTester::testServerCertificate()
{
    return  "-----BEGIN CERTIFICATE-----\n"
            "MIIDaDCCAlCgAwIBAgIJAO8vBu8i8exWMA0GCSqGSIb3DQEBCwUAMEkxCzAJBgNV\n"
            "BAYTAlVTMQswCQYDVQQIDAJDQTEtMCsGA1UEBwwkTG9zIEFuZ2VsZXNPPUJlYXN0\n"
            "Q049d3d3LmV4YW1wbGUuY29tMB4XDTE3MDUwMzE4MzkxMloXDTQ0MDkxODE4Mzkx\n"
            "MlowSTELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMS0wKwYDVQQHDCRMb3MgQW5n\n"
            "ZWxlc089QmVhc3RDTj13d3cuZXhhbXBsZS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\n"
            "A4IBDwAwggEKAoIBAQDJ7BRKFO8fqmsEXw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcF\n"
            "xqGitbnLIrOgiJpRAPLy5MNcAXE1strVGfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7b\n"
            "Fu8TsCzO6XrxpnVtWk506YZ7ToTa5UjHfBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO\n"
            "9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wWKIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBp\n"
            "yY8anC8u4LPbmgW0/U31PH0rRVfGcBbZsAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrv\n"
            "enu2tOK9Qx6GEzXh3sekZkxcgh+NlIxCNxu//Dk9AgMBAAGjUzBRMB0GA1UdDgQW\n"
            "BBTZh0N9Ne1OD7GBGJYz4PNESHuXezAfBgNVHSMEGDAWgBTZh0N9Ne1OD7GBGJYz\n"
            "4PNESHuXezAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCmTJVT\n"
            "LH5Cru1vXtzb3N9dyolcVH82xFVwPewArchgq+CEkajOU9bnzCqvhM4CryBb4cUs\n"
            "gqXWp85hAh55uBOqXb2yyESEleMCJEiVTwm/m26FdONvEGptsiCmF5Gxi0YRtn8N\n"
            "V+KhrQaAyLrLdPYI7TrwAOisq2I1cD0mt+xgwuv/654Rl3IhOMx+fKWKJ9qLAiaE\n"
            "fQyshjlPP9mYVxWOxqctUdQ8UnsUKKGEUcVrA08i1OAnVKlPFjKBvk+r7jpsTPcr\n"
            "9pWXTO9JrYMML7d+XRSZA1n3856OqZDX4403+9FnXCvfcLZLLKTBvwwFgEFGpzjK\n"
            "UEVbkhd5qstF6qWK\n"
            "-----END CERTIFICATE-----\n";
}

const std::string ConnectivityTester::testServerKey()
{
    return "-----BEGIN PRIVATE KEY-----\n"
           "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDJ7BRKFO8fqmsE\n"
           "Xw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcFxqGitbnLIrOgiJpRAPLy5MNcAXE1strV\n"
           "GfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7bFu8TsCzO6XrxpnVtWk506YZ7ToTa5UjH\n"
           "fBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wW\n"
           "KIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBpyY8anC8u4LPbmgW0/U31PH0rRVfGcBbZ\n"
           "sAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrvenu2tOK9Qx6GEzXh3sekZkxcgh+NlIxC\n"
           "Nxu//Dk9AgMBAAECggEBAK1gV8uETg4SdfE67f9v/5uyK0DYQH1ro4C7hNiUycTB\n"
           "oiYDd6YOA4m4MiQVJuuGtRR5+IR3eI1zFRMFSJs4UqYChNwqQGys7CVsKpplQOW+\n"
           "1BCqkH2HN/Ix5662Dv3mHJemLCKUON77IJKoq0/xuZ04mc9csykox6grFWB3pjXY\n"
           "OEn9U8pt5KNldWfpfAZ7xu9WfyvthGXlhfwKEetOuHfAQv7FF6s25UIEU6Hmnwp9\n"
           "VmYp2twfMGdztz/gfFjKOGxf92RG+FMSkyAPq/vhyB7oQWxa+vdBn6BSdsfn27Qs\n"
           "bTvXrGe4FYcbuw4WkAKTljZX7TUegkXiwFoSps0jegECgYEA7o5AcRTZVUmmSs8W\n"
           "PUHn89UEuDAMFVk7grG1bg8exLQSpugCykcqXt1WNrqB7x6nB+dbVANWNhSmhgCg\n"
           "VrV941vbx8ketqZ9YInSbGPWIU/tss3r8Yx2Ct3mQpvpGC6iGHzEc/NHJP8Efvh/\n"
           "CcUWmLjLGJYYeP5oNu5cncC3fXUCgYEA2LANATm0A6sFVGe3sSLO9un1brA4zlZE\n"
           "Hjd3KOZnMPt73B426qUOcw5B2wIS8GJsUES0P94pKg83oyzmoUV9vJpJLjHA4qmL\n"
           "CDAd6CjAmE5ea4dFdZwDDS8F9FntJMdPQJA9vq+JaeS+k7ds3+7oiNe+RUIHR1Sz\n"
           "VEAKh3Xw66kCgYB7KO/2Mchesu5qku2tZJhHF4QfP5cNcos511uO3bmJ3ln+16uR\n"
           "GRqz7Vu0V6f7dvzPJM/O2QYqV5D9f9dHzN2YgvU9+QSlUeFK9PyxPv3vJt/WP1//\n"
           "zf+nbpaRbwLxnCnNsKSQJFpnrE166/pSZfFbmZQpNlyeIuJU8czZGQTifQKBgHXe\n"
           "/pQGEZhVNab+bHwdFTxXdDzr+1qyrodJYLaM7uFES9InVXQ6qSuJO+WosSi2QXlA\n"
           "hlSfwwCwGnHXAPYFWSp5Owm34tbpp0mi8wHQ+UNgjhgsE2qwnTBUvgZ3zHpPORtD\n"
           "23KZBkTmO40bIEyIJ1IZGdWO32q79nkEBTY+v/lRAoGBAI1rbouFYPBrTYQ9kcjt\n"
           "1yfu4JF5MvO9JrHQ9tOwkqDmNCWx9xWXbgydsn/eFtuUMULWsG3lNjfst/Esb8ch\n"
           "k5cZd6pdJZa4/vhEwrYYSuEjMCnRb0lUsm7TsHxQrUd6Fi/mUuFU/haC0o0chLq7\n"
           "pVOUFq5mW8p0zbtfHbjkgxyF\n"
           "-----END PRIVATE KEY-----\n";
}

const std::string ConnectivityTester::testServerDH()
{
    return "-----BEGIN DH PARAMETERS-----\n"
           "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
           "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
           "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
           "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
           "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
           "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
           "-----END DH PARAMETERS-----\n";
}

void ConnectivityTester::startTestServer()
{
    m_testServer.reset(new SecuredTcpServer(m_ioService));
    m_testServer->setAddress(kConnectivityTestIp);
    m_testServer->setPort(kConnectivityTestPort);
    m_testServer->loadRawCetificate(
                ConnectivityTester::testServerCertificate(),
                ConnectivityTester::testServerKey(),
                ConnectivityTester::testServerDH());

    m_testServer->startFailed.connect([](SecuredTcpServer*){
        mainLog()->debug("failed to start connectivity test server");
    });

    m_testServer->acceptFailed.connect([](SecuredTcpServer*){
        mainLog()->debug("failed to accept a connectivity test client");
    });

    m_testServer->start();
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

    m_testDelegatee = new TestDelegatee(m_ioService, batchSize);
    m_testDelegatee->done.connect(std::bind(&ConnectivityTester::onTestDelegateeDone, this, std::placeholders::_1, std::placeholders::_2));
    m_testDelegatee->start(testIpList);
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

            if (!successfulIp.empty()) {
                mainLog()->debug("successful report: dest = {}, ips = {}", *screenId, successfulIp);
            }

            if (!failedIp.empty()) {
                mainLog()->debug("failed report: dest = {}, ips = {}", *screenId, failedIp);
            }

            newReportGenerated(*screenId, successfulIp, failedIp);

            // update connectivity results
            if (!successfulIp.empty()) {
                m_screenSuccessfulResults[*screenId] = successfulIpList;
            }
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

    testBatchFinished();

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
