#include "catch.hpp"

#include "synergy/common/UserConfig.h"
#include "synergy/common/DirectoryManager.h"

#include <boost/filesystem.hpp>

TEST_CASE("User configuration settings", "[configuration]")
{
    // test configuration file
#ifdef _WIN32
        boost::filesystem::path dir(DirectoryManager::instance()->systemAppDir());
#else
        boost::filesystem::path dir(DirectoryManager::instance()->profileDir());
#endif

    boost::filesystem::path file("synergy-test-user.cfg");
    boost::filesystem::path filename = dir / file;
    std::string testFilename = filename.string();

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

    // save to file
    boost::filesystem::remove(testFilename);
    userConfig.save(testFilename);

    // initializaiton from previous config file
    UserConfig newUserConfig;
    newUserConfig.load(testFilename);

    REQUIRE(newUserConfig.debugLevel() == kDebug);
    REQUIRE(newUserConfig.userToken() == "TestToken");
    REQUIRE(newUserConfig.userId() == 10);
    REQUIRE(newUserConfig.profileId() == 1);
    REQUIRE(newUserConfig.screenId() == 5);
    REQUIRE(newUserConfig.dragAndDrop() == true);

    // clean up
    boost::filesystem::remove(testFilename);
}
