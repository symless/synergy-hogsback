#include <synergy/common/ConfigGen.h>
#include <catch.hpp>

TEST_CASE( "Core config generation works correctly", "[ConfigGen]" ) {
    std::vector<Screen> screens;

    SECTION ("Trivial screen configurations") {
        SECTION ("An empty screens list results in an empty screen links list") {
            auto links = linkScreens (screens);
            REQUIRE (links.empty());
        };

        Screen s1;
        s1.id = 1;
        s1.width = 1920;
        s1.height = 1080;
        screens.push_back (s1);
        REQUIRE (s1.x == 0);
        REQUIRE (s1.y == 0);

        SECTION ("A single screen has no links") {
            auto links = linkScreens (screens);
            REQUIRE (links.size() == 1);
            REQUIRE (links[0].empty());
        };

        SECTION ("Tests with two screens of the same size") {
            Screen s2;
            s2.id = 2;
            s2.height = 1920;
            s2.width = 1080;
            REQUIRE (s2.x == 0);
            REQUIRE (s2.y == 0);

            SECTION ("Two screens exactly on top of one another have all "
                     "4 edges linked") {
                screens.push_back (s2);
                auto links = linkScreens (screens);
                REQUIRE (links.size() == 2);
                REQUIRE (links[0].size() == 4);
                REQUIRE (links[1].size() == 4);
            };

            SECTION ("Two screens next to one another touching horizontally "
                     "with no vertical offset") {
                s2.x = s1.x + s1.width;
                s2.y = s1.y;
                screens.push_back (s2);
                auto links = linkScreens (screens);
                REQUIRE (links.size() == 2);

                REQUIRE (links[0].size() == 1);
                REQUIRE (links[0].right().iterative_size() == 1);

                REQUIRE (links[1].size() == 1);
                REQUIRE (links[1].left().iterative_size() == 1);
            }
        }
    };
}
