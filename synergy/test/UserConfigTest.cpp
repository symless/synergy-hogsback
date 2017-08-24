#include "catch.hpp"

#include "synergy/common/UserConfig.h"
#include "synergy/common/DirectoryManager.h"

#include <boost/filesystem.hpp>

TEST_CASE("User configuration settings", "[configuration]")
{
    // test configuration file
    boost::filesystem::path dir(DirectoryManager::instance()->profileDir());
    boost::filesystem::path file("synergy-test-user.cfg");
    boost::filesystem::path filename = dir / file;
    std::string testFilename = filename.string();

    // no config file from beginning
    boost::filesystem::remove(testFilename);

    // initialization without a config file
    UserConfig userConfig(testFilename);
    userConfig.load();

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

    // save to file
    userConfig.save();

    // initializaiton from previous config file
    UserConfig newUserConfig(testFilename);
    newUserConfig.load();

    REQUIRE(newUserConfig.debugLevel() == kDebug);
    REQUIRE(newUserConfig.userToken() == "TestToken");
    REQUIRE(newUserConfig.userId() == 10);
    REQUIRE(newUserConfig.profileId() == 1);
    REQUIRE(newUserConfig.screenId() == 5);
    REQUIRE(newUserConfig.dragAndDrop() == true);

    // clean up
    boost::filesystem::remove(testFilename);
}
