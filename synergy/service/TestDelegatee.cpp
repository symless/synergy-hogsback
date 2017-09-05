#include "TestDelegatee.h"

static const long kTimeoutSec = 3;
static const std::string kDefaultConnectivityTestPort = "24810";

TestDelegatee::TestDelegatee(boost::asio::io_service &io, std::vector<std::string> &ipList, int batchSize) :
    m_ioService(io),
    m_timeout(io),
    m_ipList(std::move(ipList)),
    m_batchSize(batchSize)
{

}

void TestDelegatee::start()
{
    for (auto ip : m_ipList) {
        std::unique_ptr<SecuredTcpSession> session = std::make_unique<SecuredTcpSession>(m_ioService);

        session->setHostname(ip);
        session->setPort(kDefaultConnectivityTestPort);

        session->connected.connect(
            [this](SecuredTcpSession* orginalSession) {
                onSessionConnected(orginalSession);
            },
            boost::signals2::at_front
        );

        session->connect();

        m_sessions.push_back(std::move(session));
    }

    m_timeout.cancel();
    m_timeout.expires_from_now(boost::posix_time::seconds(kTimeoutSec));

    m_timeout.async_wait([this](const boost::system::error_code&) {
        onTestFinish();
    });
}

void TestDelegatee::onTestFinish()
{
    done(m_results, m_batchSize);
}

void TestDelegatee::onSessionConnected(SecuredTcpSession* orginalSession)
{
    auto remoteIp = orginalSession->stream().lowest_layer().remote_endpoint().address().to_string();
    m_results[remoteIp] = true;
}
