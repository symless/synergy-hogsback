#include "catch.hpp"
#include "synergy/common/UserConfig.h"
#include "synergy/common/DirectoryManager.h"
#include <boost/filesystem.hpp>

TEST_CASE("User configuration settings", "[UserConfig]")
{
    // test default config
    UserConfig userConfig;
    REQUIRE(userConfig.debugLevel() == kInfo);
    REQUIRE(userConfig.userToken() == "");
    REQUIRE(userConfig.userId() == -1);
    REQUIRE(userConfig.profileId() == -1);
    REQUIRE(userConfig.screenId() == -1);
    REQUIRE(userConfig.dragAndDrop() == false);

    // update config variables
    userConfig.setDebugLevel(kDebug);
    userConfig.setUserToken("TestToken");
    userConfig.setUserId(10);
    userConfig.setProfileId(1);
    userConfig.setScreenId(5);
    userConfig.setDragAndDrop(true);

    std::stringstream stream;
    userConfig.save(stream);

    UserConfig newUserConfig;
    stream.seekg(0, std::ios::beg);
    newUserConfig.load(stream);

    REQUIRE(newUserConfig.debugLevel() == kDebug);
    REQUIRE(newUserConfig.userToken() == "TestToken");
    REQUIRE(newUserConfig.userId() == 10);
    REQUIRE(newUserConfig.profileId() == 1);
    REQUIRE(newUserConfig.screenId() == 5);
    REQUIRE(newUserConfig.dragAndDrop() == true);
}
