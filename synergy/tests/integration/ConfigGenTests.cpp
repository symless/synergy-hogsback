#include <synergy/common/ConfigGen.h>
#include <catch.hpp>
#include <sstream>
#include <iostream>
#include <boost/filesystem.hpp>

using namespace boost;

TEST_CASE("Core config file is written to file system", "[ConfigGen]" ) {
    std::vector<Screen> screens;

    SECTION ("Save config file to file system") {
        Screen s1 (1);
        s1.name ("Bob");
        s1.width (1920);
        s1.height (1080);
        screens.push_back (s1);
        REQUIRE (s1.x() == 0);
        REQUIRE (s1.y() == 0);

        Screen s2 (2);
        s2.name ("Alice");
        s2.width (1920);
        s2.height (1080);
        screens.push_back (s2);
        REQUIRE (s1.x() == 0);
        REQUIRE (s1.y() == 0);

        auto path = filesystem::temp_directory_path() / "synergy-test-config.cfg";
        createConfigFile(path.string(), screens);

        auto expected =
            "section: screens\n"
            "  Bob:\n"
            "    halfDuplexCapsLock = false\n"
            "    halfDuplexNumLock = false\n"
            "    halfDuplexScrollLock = false\n"
            "    xtestIsXineramaUnaware = false\n"
            "    switchCorners = none\n"
            "    switchCornerSize = 0\n"
            "  Alice:\n"
            "    halfDuplexCapsLock = false\n"
            "    halfDuplexNumLock = false\n"
            "    halfDuplexScrollLock = false\n"
            "    xtestIsXineramaUnaware = false\n"
            "    switchCorners = none\n"
            "    switchCornerSize = 0\n"
            "end\n"
            "\n"
            "section: links\n"
            "  Bob:\n"
            "    up(0,100) = Alice(0,100)\n"
            "    right(0,100) = Alice(0,100)\n"
            "    down(0,100) = Alice(0,100)\n"
            "    left(0,100) = Alice(0,100)\n"
            "  Alice:\n"
            "    up(0,100) = Bob(0,100)\n"
            "    right(0,100) = Bob(0,100)\n"
            "    down(0,100) = Bob(0,100)\n"
            "    left(0,100) = Bob(0,100)\n"
            "end\n"
            "\n"
            "section: options\n"
            "  relativeMouseMoves = false\n"
            "  screenSaverSync = true\n"
            "  win32KeepForeground = false\n"
            "  switchCorners = none\n"
            "  switchCornerSize = 0\n"
            "end\n";

        std::fstream in(path.string(), std::fstream::in);
        std::stringstream ss;
        ss << in.rdbuf();
        REQUIRE (expected == ss.str());
    };
}
