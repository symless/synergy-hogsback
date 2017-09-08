#include "TestDelegatee.h"

static const long kTimeoutSec = 3;
static const std::string kDefaultConnectivityTestPort = "24810";

TestDelegatee::TestDelegatee(boost::asio::io_service &io,  int batchSize) :
    m_ioService(io),
    m_timeout(io),
    m_batchSize(batchSize)
{

}

TestDelegatee::~TestDelegatee()
{
    for (auto& tcpClient : m_tcpClients) {
        tcpClient.release();
    }
}

void TestDelegatee::start(std::vector<std::string> &ipList)
{
    for (auto ip : ipList) {
        std::unique_ptr<SecuredTcpClient> tcpClient = std::make_unique<SecuredTcpClient>(m_ioService, ip, kDefaultConnectivityTestPort);

        tcpClient->connected.connect(
            [this](SecuredTcpClient* orginalSession) {
                auto remoteIp = orginalSession->stream().lowest_layer().remote_endpoint().address().to_string();
                m_results[remoteIp] = true;
            },
            boost::signals2::at_front
        );

        tcpClient->connect();

        m_tcpClients.push_back(std::move(tcpClient));
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
