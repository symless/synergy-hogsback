#include "catch.hpp"
#include "synergy/service/ConnectivityTester.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>

static boost::signals2::signal<void()> testFinished;

TEST_CASE("Connectivity test finds a service", "[ConnectivityTester]")
{
    std::string jsonMock = R"JSON({"profile":{"id":1,"name":"default","server":1,"configVersion":0},"screens":[{"id":1,"name":"foo","x_pos":100,"y_pos":200,"active":true,"status":"Connecting","ipList":"192.168.3.1,127.0.0.1"},{"id":2,"name":"bar","x_pos":200,"y_pos":200,"active":true,"status":"Connecting","ipList":"192.168.1.1"}]})JSON";

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

    std::shared_ptr<ProfileConfig> localProfileConfig = std::make_shared<ProfileConfig>();
    ConnectivityTester tester(ioService, localProfileConfig);
    tester.testBatchFinished.connect([](){
        testFinished();
    });

    try {
        ProfileConfig profileConfig = ProfileConfig::fromJSONSnapshot(jsonMock);
        localProfileConfig->apply(profileConfig);

        ioService.run();

        std::vector<std::string> result = tester.getSuccessfulResults(1);

        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == "127.0.0.1");
    }
    catch (...) {
        REQUIRE("Parsing JSON Okey" == "Parsing JSON failed");
        return;
    }
}
