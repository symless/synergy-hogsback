#include <synergy/common/ProfileConfig.h>
#include <catch.hpp>

TEST_CASE("Profile config", "[ProfileConfig]" ) {

    SECTION ("Compare with add") {
        std::string jsonMock1 = R"JSON({"profile":{"id":1,"name":"mock1","server":1,"configVersion":1},"screens":[{"id":1,"name":"foo1","x_pos":100,"y_pos":200,"version":1,"active":true,"status":"Connecting","ipList":"192.168.3.1,127.0.0.1","error_code":0,"error_message":""}]})JSON";
        std::string jsonMock2 = R"JSON({"profile":{"id":2,"name":"mock2","server":2,"configVersion":1},"screens":[{"id":1,"name":"foo2","x_pos":101,"y_pos":201,"version":1,"active":false,"status":"Connected","ipList":"192.168.3.2,127.0.0.1","error_code":0,"error_message":""},{"id":2,"name":"bar","x_pos":200,"y_pos":200,"version":1,"active":true,"status":"Connecting","ipList":"192.168.1.1","error_code":0,"error_message":""}]})JSON";

        ProfileConfig profileConfig1 = ProfileConfig::fromJsonSnapshot(jsonMock1);
        ProfileConfig profileConfig2 = ProfileConfig::fromJsonSnapshot(jsonMock2);

        // TODO: also assert that all signals get called
        REQUIRE(profileConfig1.compare(profileConfig2) == true);
    }

    SECTION ("Compare with remove") {
        std::string jsonMock1 = R"JSON({"profile":{"id":2,"name":"mock2","server":2,"configVersion":1},"screens":[{"id":1,"name":"foo2","x_pos":101,"y_pos":201,"version":1,"active":false,"status":"Connected","ipList":"192.168.3.2,127.0.0.1","error_code":0,"error_message":""},{"id":2,"name":"bar","x_pos":200,"y_pos":200,"version":1,"active":true,"status":"Connecting","ipList":"192.168.1.1","error_code":0,"error_message":""}]})JSON";
        std::string jsonMock2 = R"JSON({"profile":{"id":1,"name":"mock1","server":1,"configVersion":1},"screens":[{"id":1,"name":"foo1","x_pos":100,"y_pos":200,"version":1,"active":true,"status":"Connecting","ipList":"192.168.3.1,127.0.0.1","error_code":0,"error_message":""}]})JSON";

        ProfileConfig profileConfig1 = ProfileConfig::fromJsonSnapshot(jsonMock1);
        ProfileConfig profileConfig2 = ProfileConfig::fromJsonSnapshot(jsonMock2);

        // TODO: also assert that all signals get called
        REQUIRE(profileConfig1.compare(profileConfig2) == true);
    }
}
