#include <synergy/common/UserConfig.h>
#include <catch.hpp>
#include <sstream>
#include <iostream>

TEST_CASE("User config read and write", "[UserConfig]" ) {

    UserConfig userConfig;
    std::stringstream ss;

    SECTION ("Read config file") {

        ss << "[auth]" << std::endl;
        ss << "user-id = 1" << std::endl;
        ss << "user-token = \"mock\"" << std::endl;

        userConfig.load(ss);

        REQUIRE(userConfig.userId() == 1);
        REQUIRE(userConfig.userToken() == "mock");
        REQUIRE(userConfig.versionCheck() == true);
    }

    SECTION ("Write config file") {

        userConfig.setSystemUid("mock");
        userConfig.save(ss);

        std::string section1, value1;
        std::getline(ss, section1);
        std::getline(ss, value1);

        REQUIRE(section1 == "[system]");
        REQUIRE(value1 == "	uid = \"mock\"");
    }

    SECTION ("Read config file with developer section") {

        ss << "[developer]" << std::endl;
        ss << "version-check = false" << std::endl;

        userConfig.load(ss);

        REQUIRE(userConfig.versionCheck() == false);
    }

    SECTION ("Read and write config file with developer section") {

        ss << "[developer]" << std::endl;
        ss << "version-check = false" << std::endl;

        userConfig.load(ss);

        std::stringstream ss2;
        userConfig.save(ss2);

        std::string section1, value1;
        std::getline(ss2, section1);
        std::getline(ss2, value1);

        REQUIRE(section1 == "[developer]");
        REQUIRE(value1 == "	version-check = false");
    }
}
