#include <catch.hpp>
#include <synergy/common/ProcessCommand.h>
#include <synergy/common/DirectoryManager.h>
#include <boost/algorithm/string/join.hpp>

TEST_CASE("Process command and args generated correctly", "[ProcessCommand]")
{
    ProcessCommand pc;

    // TODO: stub out profile directory
    auto profileDir = DirectoryManager::instance()->profileDir();
    auto installDir = DirectoryManager::instance()->installDir();
    auto systemLogDir = DirectoryManager::instance()->systemLogDir();
    auto configPath = profileDir / "synergy.conf";
    auto corePath = installDir / kCoreProgram;
    auto logPath = systemLogDir / kCoreLogFile;

#ifdef __APPLE__
    corePath = corePath.parent_path() / "Resources" / kCoreProgram;
#endif

    SECTION("Server command line")
    {
        auto command = pc.generate(true, "mock local hostname");

        // TODO: test each element instead of using string join
        REQUIRE(boost::algorithm::join(command, " ") ==
            corePath.string() + " --server -f "
            "--debug " + kCoreDebugLevel + " "
            "--name mock local hostname --enable-drag-drop "
            "--profile-dir " + profileDir.string() + " "
            "--log " + logPath.string() + " "
            "-c " + configPath.string() + " "
            "--address 127.0.0.1:24800");
    }

    SECTION("Client command line")
    {
        auto command = pc.generate(false, "mock local hostname");

        // TODO: test each element instead of using string join
        REQUIRE(boost::algorithm::join(command, " ") ==
            corePath.string() + " --client -f "
            "--debug " + kCoreDebugLevel + " "
            "--name mock local hostname --enable-drag-drop "
            "--profile-dir " + profileDir.string() + " "
            "--log " + logPath.string() + " 127.0.0.1:24801");
    }
}
