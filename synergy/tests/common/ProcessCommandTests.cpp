#include <catch.hpp>
#include <synergy/common/ProcessCommand.h>
#include <synergy/common/DirectoryManager.h>
#include <boost/algorithm/string/join.hpp>

TEST_CASE("Process command and args generated correctly", "[ProcessCommand]")
{
    ProcessCommand pc;
    pc.setLocalHostname("mock local hostname");

    // TODO: stub out profile directory
    auto profileDir = DirectoryManager::instance()->profileDir();
    auto installDir = DirectoryManager::instance()->installedDir();
    auto configPath = profileDir / "synergy.conf";
    auto serverPath = installDir / kServerCmd;
    auto clientPath = installDir / kClientCmd;

    SECTION("Server command line")
    {
        auto command = pc.generate(true);

        // TODO: test each element instead of using string join
        REQUIRE(boost::algorithm::join(command, " ") ==
            serverPath.string() + " -f --no-tray --debug DEBUG "
            "--name mock local hostname --enable-drag-drop "
            "--profile-dir " + profileDir.string() + " "
            "--log synergy.log -c " + configPath.string() + " "
            "--address :24800");
    }

    SECTION("Client command line")
    {
        pc.setServerAddress("mock server address");
        auto command = pc.generate(false);

        // TODO: test each element instead of using string join
        REQUIRE(boost::algorithm::join(command, " ") ==
            clientPath.string() + " -f --no-tray --debug DEBUG "
            "--name mock local hostname --enable-drag-drop "
            "--profile-dir " + profileDir.string() + " "
            "--log synergy.log mock server address:24800");
    }
}
