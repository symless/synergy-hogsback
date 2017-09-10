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
    auto configPath = profileDir / "synergy.conf";

    SECTION("Server command line")
    {
        auto command = pc.generate(true);

        REQUIRE(boost::algorithm::join(command, " ") ==
            kServerCmd + " -f --no-tray --debug DEBUG "
            "--name \"mock local hostname\" --enable-drag-drop "
            "--profile-dir \"" + profileDir.string() + "\" "
            "--log synergy.log -c \"" + configPath.string() + "\" "
            "--address :24800");
    }

    SECTION("Client command line")
    {
        pc.setServerAddress("mock server address");
        auto command = pc.generate(false);

        REQUIRE(boost::algorithm::join(command, " ") ==
            kClientCmd + " -f --no-tray --debug DEBUG "
            "--name \"mock local hostname\" --enable-drag-drop "
            "--profile-dir \"" + profileDir.string() + "\" "
            "--log synergy.log \"mock server address:24800\"");
    }
}
