#include "catch.hpp"

#include "synergy/service/ConnectivityTester.h"
#include "synergy/service/SecuredTcpServer.h"
#include "synergy/service/SecuredTcpClient.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>

static boost::signals2::signal<void()> testFinished;

TEST_CASE("Secured TCP Connection test", "[SecuredTcp]")
{
    const std::string kServerTestIp = "0.0.0.0";
    const std::string kLocalIp = "127.0.0.1";
    const std::string kTestPort = "24811";
    bool connected = false;

    boost::asio::io_service ioService;

    SecuredTcpServer server(ioService);
    server.setAddress(kServerTestIp);
    server.setPort(kTestPort);
    server.loadRawCetificate(
                ConnectivityTester::testServerCertificate(),
                ConnectivityTester::testServerKey(),
                ConnectivityTester::testServerDH());

    SecuredTcpClient client(ioService, kLocalIp, kTestPort);

    server.startFailed.connect([](SecuredTcpServer*){
        testFinished();
    });

    server.acceptFailed.connect([](SecuredTcpServer*){
        testFinished();
    });

    client.connected.connect([&connected](SecuredTcpClient*){
        connected = true;
        testFinished();
    });

    client.connectFailed.connect([](SecuredTcpClient*){
        testFinished();
    });

    testFinished.connect([&ioService]() {
        ioService.poll();
        ioService.stop();
    });

    boost::asio::deadline_timer timer(ioService);
    timer.expires_from_now(boost::posix_time::seconds(3));
    timer.async_wait([](const boost::system::error_code&) {
        testFinished();
    });

    server.start();
    client.connect();

    ioService.run();

    REQUIRE(connected == true);
}
