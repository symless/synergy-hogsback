#include "catch.hpp"
#include <synergy/service/ConnectivityTester.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>

static boost::signals2::signal<void()> testFinished;

TEST_CASE("Connectivity test finds a service", "[ConnectivityTester]")
{
    std::string jsonMock = R"JSON({"id":1,"configVersion":0,"server":1,"name":"default","screens":[{"id":1,"name":"foo","x_pos":100,"y_pos":200,"ipList":"192.168.3.1,127.0.0.1","status":"connecting","active":true},{"id":2,"name":"bar","x_pos":200,"y_pos":200,"ipList":"192.168.1.1","status":"connecting","active":true}]})JSON";
    std::shared_ptr<Profile> profile = std::make_shared<Profile>(-1);

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

    ConnectivityTester tester(ioService, profile);
    tester.testBatchFinished.connect([](){
        testFinished();
    });

    profile->apply(Profile::fromJSONSnapshot(jsonMock));

    ioService.run();

    std::vector<std::string> result = tester.getSuccessfulResults(1);

    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == "127.0.0.1");
}
