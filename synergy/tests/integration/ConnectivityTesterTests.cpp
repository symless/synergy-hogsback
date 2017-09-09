#include "catch.hpp"
#include "synergy/service/ConnectivityTester.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>

static boost::signals2::signal<void()> testFinished;

TEST_CASE("Connectivity test finds a service", "[ConnectivityTester]")
{
    std::string jsonMock = R"JSON({"profile":{"configVersion":0,"id":1,"name":"default","server":1},"screens":[{"id":1,"name":"foo","active":true,"ipList":"192.168.3.1,127.0.0.1","status":"connecting","x_pos":100,"y_pos":200},{"id":2,"name":"bar","active":true,"ipList":"192.168.1.1","status":"connecting","x_pos":200,"y_pos":200}]})JSON";
    ProfileSnapshot profileSnapshot;
    profileSnapshot.parseJsonSnapshot(jsonMock);

    boost::asio::io_service ioService;

    testFinished.connect([&ioService]() {
        ioService.poll();
        ioService.stop();
    });

    boost::asio::deadline_timer timer(ioService);
    timer.expires_from_now(boost::posix_time::seconds(5));
    timer.async_wait([](const boost::system::error_code&) {
        testFinished();
    });

    ConnectivityTester tester(ioService);
    tester.testBatchFinished.connect([](){
        testFinished();
    });

    tester.testNewScreens(profileSnapshot.getScreens());

    ioService.run();

    std::vector<std::string> result = tester.getSuccessfulResults(1);

    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == "127.0.0.1");
}
