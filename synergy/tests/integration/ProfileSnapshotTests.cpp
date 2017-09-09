#include "catch.hpp"
#include "synergy/service/ProfileSnapshot.h"

TEST_CASE("Deserialize profile from JSON", "[ProfileSnapshot]")
{
    std::string jsonMock = R"JSON({"profile":{"configVersion":0,"id":1,"name":"default","server":1},"screens":[{"id":1,"name":"foo","active":true,"ipList":"192.168.1.1","status":"connecting","x_pos":100,"y_pos":200},{"id":2,"name":"bar","active":false,"ipList":"192.168.1.2","status":"connecting","x_pos":200,"y_pos":200}]})JSON";

    ProfileSnapshot profileSnapshot;
    profileSnapshot.parseJsonSnapshot(jsonMock);
}
