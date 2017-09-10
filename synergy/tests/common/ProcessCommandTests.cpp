#include <catch.hpp>
#include <synergy/common/ProcessCommand.h>
#include <synergy/common/DirectoryManager.h>

TEST_CASE("Process command and args generated correctly", "[ProcessCommand]")
{
    ProcessCommand pc;
    pc.setLocalHostname("mock local hostname");

    // TODO: stub out profile directory
    std::string profileDir = DirectoryManager::instance()->profileDir().string();

    SECTION("Server command line")
    {
        auto command = pc.print(true);
        REQUIRE(command ==
            kServerCmd + " -f --no-tray --debug DEBUG "
            "--name \"mock local hostname\" --enable-drag-drop "
            "--profile-dir \"" + profileDir + "\" "
            "--log synergy.log -c \"" + profileDir + "\\synergy.conf\" "
            "--address :24800");
    }

    SECTION("Client command line")
    {
        pc.setServerAddress("mock server address");
        auto command = pc.print(false);

        REQUIRE(command ==
            kClientCmd + " -f --no-tray --debug DEBUG "
            "--name \"mock local hostname\" --enable-drag-drop "
            "--profile-dir \"" + profileDir + "\" "
            "--log synergy.log \"mock server address:24800\"");
    }
}
