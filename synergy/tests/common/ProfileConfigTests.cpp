#include <synergy/common/ProfileConfig.h>
#include <catch.hpp>

TEST_CASE("Profile config", "[ProfileConfig]" ) {

    SECTION ("Compare with add") {
        std::string jsonMock1 = R"JSON({"profile":{"id":1,"name":"mock1","server":1,"configVersion":1},"screens":[{"id":1,"name":"foo1","x_pos":100,"y_pos":200,"active":true,"status":"Connecting","ipList":"192.168.3.1,127.0.0.1"}]})JSON";
        std::string jsonMock2 = R"JSON({"profile":{"id":2,"name":"mock2","server":2,"configVersion":1},"screens":[{"id":1,"name":"foo2","x_pos":101,"y_pos":201,"active":false,"status":"Connected","ipList":"192.168.3.2,127.0.0.1"},{"id":2,"name":"bar","x_pos":200,"y_pos":200,"active":true,"status":"Connecting","ipList":"192.168.1.1"}]})JSON";

        ProfileConfig profileConfig1 = ProfileConfig::fromJSONSnapshot(jsonMock1);
        ProfileConfig profileConfig2 = ProfileConfig::fromJSONSnapshot(jsonMock2);

        profileConfig1.updateScreenTestResult(1, "1.1.1.1", "2.2.2.2");
        profileConfig2.updateScreenTestResult(1, "3.3.3.3", "4.4.4.4");

        // TODO: also assert that all signals get called
        REQUIRE(profileConfig1.compare(profileConfig2) == true);
    }

    SECTION ("Compare with remove") {
        std::string jsonMock1 = R"JSON({"profile":{"id":2,"name":"mock2","server":2,"configVersion":1},"screens":[{"id":1,"name":"foo2","x_pos":101,"y_pos":201,"active":false,"status":"Connected","ipList":"192.168.3.2,127.0.0.1"},{"id":2,"name":"bar","x_pos":200,"y_pos":200,"active":true,"status":"Connecting","ipList":"192.168.1.1"}]})JSON";
        std::string jsonMock2 = R"JSON({"profile":{"id":1,"name":"mock1","server":1,"configVersion":1},"screens":[{"id":1,"name":"foo1","x_pos":100,"y_pos":200,"active":true,"status":"Connecting","ipList":"192.168.3.1,127.0.0.1"}]})JSON";

        ProfileConfig profileConfig1 = ProfileConfig::fromJSONSnapshot(jsonMock1);
        ProfileConfig profileConfig2 = ProfileConfig::fromJSONSnapshot(jsonMock2);

        profileConfig1.updateScreenTestResult(1, "1.1.1.1", "2.2.2.2");
        profileConfig2.updateScreenTestResult(1, "3.3.3.3", "4.4.4.4");

        // TODO: also assert that all signals get called
        REQUIRE(profileConfig1.compare(profileConfig2) == true);
    }
}
