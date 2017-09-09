#include <synergy/common/ConfigGen.h>
#include <catch.hpp>
#include <sstream>
#include <iostream>

TEST_CASE( "Core config generation works correctly", "[ConfigGen]" ) {
    std::vector<Screen> screens;

    SECTION ("Trivial screen configurations") {
        SECTION ("An empty screens list results in an empty screen links list") {
            auto links = linkScreens (screens);
            REQUIRE (links.empty());
        };

        Screen s1 (1);
        s1.name ("Andrews-PC");
        s1.width (1920);
        s1.height (1080);
        screens.push_back (s1);
        REQUIRE (s1.x() == 0);
        REQUIRE (s1.y() == 0);

        SECTION ("A single screen has no links") {
            auto links = linkScreens (screens);
            REQUIRE (links.size() == 1);
            REQUIRE (links[0].empty());
        };

        SECTION ("Tests with two screens of the same size") {
            Screen s2 (2);
            s2.name ("Andrews-iMac");
            s2.width (1920);
            s2.height (1080);
            REQUIRE (s2.x() == 0);
            REQUIRE (s2.y() == 0);

            SECTION ("Two screens exactly on top of one another have all "
                     "4 edges linked") {
                screens.push_back (s2);
                auto links = linkScreens (screens);
                REQUIRE (links.size() == 2);
                REQUIRE (links[0].size() == 4);
                REQUIRE (links[1].size() == 4);
            };

            SECTION ("Two screens (s1,s2) next to one another, touching "
                     "horizontally, with no vertical offset") {
                s2.x(s1.x() + s1.width());
                s2.y(s1.y());
                screens.push_back (s2);
                auto links = linkScreens (screens);
                REQUIRE (links.size() == 2);

                REQUIRE (links[0].size() == 1);
                REQUIRE (links[0].left().empty());
                REQUIRE (links[0].top().empty());
                REQUIRE (links[0].bottom().empty());
                REQUIRE (links[0].right().iterative_size() == 1);

                auto link = links[0].right().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s2.id());

                REQUIRE (links[1].size() == 1);
                REQUIRE (links[1].right().empty());
                REQUIRE (links[1].top().empty());
                REQUIRE (links[1].bottom().empty());
                REQUIRE (links[1].left().iterative_size() == 1);

                link = links[1].left().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s1.id());

                std::ostringstream oss;
                printScreenLinks (oss, screens, links, 0);
                printScreenLinks (oss, screens, links, 1);
                auto expected =
                    "Andrews-PC:\n"
                        "\tright(0,100) = Andrews-iMac(0,100)\n"
                    "Andrews-iMac:\n"
                        "\tleft(0,100) = Andrews-PC(0,100)\n";
                REQUIRE (expected == oss.str());
            }

            SECTION ("Two screens (s1,s2) next to one another, touching "
                     "horizontally, with a 50% vertical offset on s2") {
                s2.x (s1.x() + s1.width());
                s2.y (s1.y() + s1.height() / 2);
                screens.push_back (s2);
                auto links = linkScreens (screens);
                REQUIRE (links.size() == 2);

                REQUIRE (links[0].size() == 1);
                REQUIRE (links[0].left().empty());
                REQUIRE (links[0].top().empty());
                REQUIRE (links[0].bottom().empty());
                REQUIRE (links[0].right().iterative_size() == 1);

                auto link = links[0].right().begin();
                REQUIRE (link->first.lower() == 540);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s2.id());

                REQUIRE (links[1].size() == 1);
                REQUIRE (links[1].right().empty());
                REQUIRE (links[1].top().empty());
                REQUIRE (links[1].bottom().empty());
                REQUIRE (links[1].left().iterative_size() == 1);

                link = links[1].left().begin();
                REQUIRE (link->first.lower() == 540);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s1.id());

                std::ostringstream oss;
                printScreenLinks (oss, screens, links, 0);
                printScreenLinks (oss, screens, links, 1);
                auto expected =
                    "Andrews-PC:\n"
                        "\tright(50,100) = Andrews-iMac(0,50)\n"
                    "Andrews-iMac:\n"
                        "\tleft(0,50) = Andrews-PC(50,100)\n";
                REQUIRE (expected == oss.str());
            }

            SECTION ("Two screens (s1,s2) next to one another, spaced "
                     "horizontally, with a 50% vertical offset on s2") {
                auto const space = 5000;
                s2.x (s1.x() + s1.width() + space);
                s2.y (s1.y() + s1.height() / 2);
                screens.push_back (s2);
                auto links = linkScreens (screens);
                REQUIRE (links.size() == 2);

                REQUIRE (links[0].size() == 1);
                REQUIRE (links[0].left().empty());
                REQUIRE (links[0].top().empty());
                REQUIRE (links[0].bottom().empty());
                REQUIRE (links[0].right().iterative_size() == 1);

                auto link = links[0].right().begin();
                REQUIRE (link->first.lower() == 540);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s2.id());

                REQUIRE (links[1].size() == 1);
                REQUIRE (links[1].right().empty());
                REQUIRE (links[1].top().empty());
                REQUIRE (links[1].bottom().empty());
                REQUIRE (links[1].left().iterative_size() == 1);

                link = links[1].left().begin();
                REQUIRE (link->first.lower() == 540);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s1.id());

                std::ostringstream oss;
                printScreenLinks (oss, screens, links, 0);
                printScreenLinks (oss, screens, links, 1);
                auto expected =
                    "Andrews-PC:\n"
                        "\tright(50,100) = Andrews-iMac(0,50)\n"
                    "Andrews-iMac:\n"
                        "\tleft(0,50) = Andrews-PC(50,100)\n";
                REQUIRE (expected == oss.str());
            }

            SECTION ("Two screens (s1,s2) of the same size positioned "
                     "diagonally are not linked") {
                s2.x (s1.x() + s1.width());
                s2.y (s1.y() + s1.height());
                screens.push_back (s2);
                auto links = linkScreens (screens);
                REQUIRE (links.size() == 2);
                REQUIRE (links[0].size() == 0);
                REQUIRE (links[1].size() == 0);

                std::ostringstream oss;
                printScreenLinks (oss, screens, links, 0);
                printScreenLinks (oss, screens, links, 1);
                auto expected =
                    "Andrews-PC:\n"
                    "Andrews-iMac:\n";
                REQUIRE (expected == oss.str());
            }

            SECTION ("Two screens (s1,s2) on top of one another, touching "
                     "vertically, with no horizontal offset") {
                s2.x (s1.x());
                s2.y (s1.y() + s1.height());
                screens.push_back (s2);
                auto links = linkScreens (screens);
                REQUIRE (links.size() == 2);

                REQUIRE (links[0].size() == 1);
                REQUIRE (links[0].left().empty());
                REQUIRE (links[0].right().empty());
                REQUIRE (links[0].top().empty());
                REQUIRE (links[0].bottom().iterative_size() == 1);

                auto link = links[0].bottom().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 1920);
                REQUIRE (link->second == s2.id());

                REQUIRE (links[1].size() == 1);
                REQUIRE (links[1].left().empty());
                REQUIRE (links[1].right().empty());
                REQUIRE (links[1].top().iterative_size() == 1);
                REQUIRE (links[1].bottom().empty());

                link = links[1].top().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 1920);
                REQUIRE (link->second == s1.id());

                std::ostringstream oss;
                printScreenLinks (oss, screens, links, 0);
                printScreenLinks (oss, screens, links, 1);
                auto expected =
                    "Andrews-PC:\n"
                        "\tdown(0,100) = Andrews-iMac(0,100)\n"
                    "Andrews-iMac:\n"
                        "\tup(0,100) = Andrews-PC(0,100)\n";
                REQUIRE (expected == oss.str());
            }

            SECTION ("Two screens (s1,s2) on top of one another, touching "
                     "vertically, with a 50% horizontal offset") {
                s2.x (s1.x() + s1.width() / 2);
                s2.y (s1.y() + s1.height());
                screens.push_back (s2);
                auto links = linkScreens (screens);
                REQUIRE (links.size() == 2);

                REQUIRE (links[0].size() == 1);
                REQUIRE (links[0].left().empty());
                REQUIRE (links[0].right().empty());
                REQUIRE (links[0].top().empty());
                REQUIRE (links[0].bottom().iterative_size() == 1);

                auto link = links[0].bottom().begin();
                REQUIRE (link->first.lower() == 960);
                REQUIRE (link->first.upper() == 1920);
                REQUIRE (link->second == s2.id());

                REQUIRE (links[1].size() == 1);
                REQUIRE (links[1].left().empty());
                REQUIRE (links[1].right().empty());
                REQUIRE (links[1].top().iterative_size() == 1);
                REQUIRE (links[1].bottom().empty());

                link = links[1].top().begin();
                REQUIRE (link->first.lower() == 960);
                REQUIRE (link->first.upper() == 1920);
                REQUIRE (link->second == s1.id());

                std::ostringstream oss;
                printScreenLinks (oss, screens, links, 0);
                printScreenLinks (oss, screens, links, 1);
                auto expected =
                    "Andrews-PC:\n"
                        "\tdown(50,100) = Andrews-iMac(0,50)\n"
                    "Andrews-iMac:\n"
                        "\tup(0,50) = Andrews-PC(50,100)\n";
                REQUIRE (expected == oss.str());
            }
        }

        SECTION ("Tests with three screens of the same size") {
            Screen s2(2);
            s2.name ("Andrews-iMac");
            s2.width (1920);
            s2.height (1080);
            REQUIRE (s2.x() == 0);
            REQUIRE (s2.y() == 0);

            Screen s3(3);
            s3.name ("Andrews-Cray2");
            s3.width (1920);
            s3.height (1080);
            REQUIRE (s3.x() == 0);
            REQUIRE (s3.y() == 0);

            SECTION ("Three screens (s1,s2,s3) next to one another, touching "
                     "horizontally, with no vertical offset") {
                s2.x (s1.x() + s1.width());
                s2.y (s1.y());
                s3.x (s1.x() + s1.width() + s2.width());
                s3.y (s1.y());
                screens.push_back (s2);
                screens.push_back (s3);
                auto links = linkScreens (screens);
                REQUIRE (links.size() == 3);

                /* Check links on screen 1 */
                REQUIRE (links[0].size() == 1);
                //REQUIRE (std::distance(links[0].begin(), links[0].end()) == 1);
                REQUIRE (links[0].left().empty());
                REQUIRE (links[0].top().empty());
                REQUIRE (links[0].bottom().empty());
                REQUIRE (links[0].right().iterative_size() == 1);

                auto link = links[0].right().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s2.id());

                /* Check links on screen 2 */
                REQUIRE (links[1].size() == 2);
                REQUIRE (links[1].top().empty());
                REQUIRE (links[1].bottom().empty());
                REQUIRE (links[1].left().iterative_size() == 1);
                REQUIRE (links[1].right().iterative_size() == 1);

                link = links[1].left().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s1.id());

                link = links[1].right().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s3.id());

                /* Check links on screen 3 */
                REQUIRE (links[2].size() == 1);
                REQUIRE (links[2].top().empty());
                REQUIRE (links[2].bottom().empty());
                REQUIRE (links[2].left().iterative_size() == 1);
                REQUIRE (links[2].right().empty());

                link = links[2].left().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s2.id());

                std::ostringstream oss;
                printScreenLinks (oss, screens, links, 0);
                printScreenLinks (oss, screens, links, 1);
                printScreenLinks (oss, screens, links, 2);
                auto expected =
                    "Andrews-PC:\n"
                        "\tright(0,100) = Andrews-iMac(0,100)\n"
                    "Andrews-iMac:\n"
                        "\tright(0,100) = Andrews-Cray2(0,100)\n"
                        "\tleft(0,100) = Andrews-PC(0,100)\n"
                    "Andrews-Cray2:\n"
                        "\tleft(0,100) = Andrews-iMac(0,100)\n";
                REQUIRE (expected == oss.str());
            }


            SECTION ("Three screens (s1,s2,s3) next to one another, touching "
                     "horizontally, with the center screen dropped by 50% "
                     "(screens added left to right)") {
                s2.x (s1.x() + s1.width());
                s2.y (s1.y() + s1.height() / 2);
                s3.x (s1.x() + s1.width() + s2.width());
                s3.y (s1.y());
                screens.push_back (s2);
                screens.push_back (s3);
                auto links = linkScreens (screens);
                REQUIRE (links.size() == 3);

                /* Check links on screen 1 */
                REQUIRE (links[0].size() == 2);
                REQUIRE (links[0].left().empty());
                REQUIRE (links[0].top().empty());
                REQUIRE (links[0].bottom().empty());
                REQUIRE (links[0].right().iterative_size() == 2);

                auto link = links[0].right().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 540);
                REQUIRE (link->second == s3.id());

                ++link;
                REQUIRE (link->first.lower() == 540);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s2.id());

                /* Check links on screen 2 */
                REQUIRE (links[1].size() == 2);
                REQUIRE (links[1].top().empty());
                REQUIRE (links[1].bottom().empty());
                REQUIRE (links[1].left().iterative_size() == 1);
                REQUIRE (links[1].right().iterative_size() == 1);

                link = links[1].left().begin();
                REQUIRE (link->first.lower() == 540);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s1.id());

                link = links[1].right().begin();
                REQUIRE (link->first.lower() == 540);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s3.id());

                /* Check links on screen 3 */
                REQUIRE (links[2].size() == 2);
                REQUIRE (links[2].top().empty());
                REQUIRE (links[2].bottom().empty());
                REQUIRE (links[2].left().iterative_size() == 2);
                REQUIRE (links[2].right().empty());

                link = links[2].left().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 540);
                REQUIRE (link->second == s1.id());

                ++link;
                REQUIRE (link->first.lower() == 540);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s2.id());

                std::ostringstream oss;
                printScreenLinks (oss, screens, links, 0);
                printScreenLinks (oss, screens, links, 1);
                printScreenLinks (oss, screens, links, 2);
                auto expected =
                    "Andrews-PC:\n"
                        "\tright(0,50) = Andrews-Cray2(0,50)\n"
                        "\tright(50,100) = Andrews-iMac(0,50)\n"
                    "Andrews-iMac:\n"
                        "\tright(0,50) = Andrews-Cray2(50,100)\n"
                        "\tleft(0,50) = Andrews-PC(50,100)\n"
                    "Andrews-Cray2:\n"
                        "\tleft(0,50) = Andrews-PC(0,50)\n"
                        "\tleft(50,100) = Andrews-iMac(0,50)\n";
                REQUIRE (expected == oss.str());
            }

            SECTION ("Three screens (s1,s2,s3) next to one another, touching "
                     "horizontally, with the center screen dropped by 50% "
                     "(screens added out of order)") {
                s2.x (s1.x() + s1.width());
                s2.y (s1.y() + s1.height() / 2);
                s3.x (s1.x() + s1.width() + s2.width());
                s3.y (s1.y());
                screens.push_back (s3);
                screens.push_back (s2);
                auto links = linkScreens (screens);
                REQUIRE (links.size() == 3);

                /* Check links on screen 1 */
                REQUIRE (links[0].size() == 2);
                REQUIRE (links[0].left().empty());
                REQUIRE (links[0].top().empty());
                REQUIRE (links[0].bottom().empty());
                REQUIRE (links[0].right().iterative_size() == 2);

                auto link = links[0].right().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 540);
                REQUIRE (link->second == s3.id());

                ++link;
                REQUIRE (link->first.lower() == 540);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s2.id());

                /* Check links on screen 3 */
                REQUIRE (links[2].size() == 2);
                REQUIRE (links[2].top().empty());
                REQUIRE (links[2].bottom().empty());
                REQUIRE (links[2].left().iterative_size() == 1);
                REQUIRE (links[2].right().iterative_size() == 1);

                link = links[2].left().begin();
                REQUIRE (link->first.lower() == 540);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s1.id());

                link = links[2].right().begin();
                REQUIRE (link->first.lower() == 540);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s3.id());

                /* Check links on screen 2 */
                REQUIRE (links[1].size() == 2);
                REQUIRE (links[1].top().empty());
                REQUIRE (links[1].bottom().empty());
                REQUIRE (links[1].left().iterative_size() == 2);
                REQUIRE (links[1].right().empty());

                link = links[1].left().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 540);
                REQUIRE (link->second == s1.id());

                ++link;
                REQUIRE (link->first.lower() == 540);
                REQUIRE (link->first.upper() == 1080);
                REQUIRE (link->second == s2.id());

                std::ostringstream oss;
                printScreenLinks (oss, screens, links, 0);
                printScreenLinks (oss, screens, links, 2);
                printScreenLinks (oss, screens, links, 1);
                auto expected =
                    "Andrews-PC:\n"
                        "\tright(0,50) = Andrews-Cray2(0,50)\n"
                        "\tright(50,100) = Andrews-iMac(0,50)\n"
                    "Andrews-iMac:\n"
                        "\tright(0,50) = Andrews-Cray2(50,100)\n"
                        "\tleft(0,50) = Andrews-PC(50,100)\n"
                    "Andrews-Cray2:\n"
                        "\tleft(0,50) = Andrews-PC(0,50)\n"
                        "\tleft(50,100) = Andrews-iMac(0,50)\n";
                REQUIRE (expected == oss.str());
            }

            SECTION ("Three screens (s1,s2,s3) on top of one another, touching "
                     "vertically, with the center screen kicked to the right by "
                     "50%") {
                s2.x (s1.x() + s1.width() / 2);
                s2.y (s1.y() + s1.height());
                s3.x (s1.x());
                s3.y (s2.y() + s2.height());
                screens.push_back (s2);
                screens.push_back (s3);
                auto links = linkScreens (screens);
                REQUIRE (links.size() == 3);

                /* Check links on screen 1 */
                REQUIRE (links[0].size() == 2);
                REQUIRE (links[0].left().empty());
                REQUIRE (links[0].top().empty());
                REQUIRE (links[0].bottom().iterative_size() == 2);
                REQUIRE (links[0].right().empty());

                auto link = links[0].bottom().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 960);
                REQUIRE (link->second == s3.id());

                ++link;
                REQUIRE (link->first.lower() == 960);
                REQUIRE (link->first.upper() == 1920);
                REQUIRE (link->second == s2.id());

                /* Check links on screen 2 */
                REQUIRE (links[1].size() == 2);
                REQUIRE (links[1].top().iterative_size() == 1);
                REQUIRE (links[1].bottom().iterative_size() == 1);
                REQUIRE (links[1].left().empty());
                REQUIRE (links[1].right().empty());

                link = links[1].top().begin();
                REQUIRE (link->first.lower() == 960);
                REQUIRE (link->first.upper() == 1920);
                REQUIRE (link->second == s1.id());

                link = links[1].bottom().begin();
                REQUIRE (link->first.lower() == 960);
                REQUIRE (link->first.upper() == 1920);
                REQUIRE (link->second == s3.id());

                /* Check links on screen 3 */
                REQUIRE (links[2].size() == 2);
                REQUIRE (links[2].top().iterative_size() == 2);
                REQUIRE (links[2].bottom().empty());
                REQUIRE (links[2].left().empty());
                REQUIRE (links[2].right().empty());

                link = links[2].top().begin();
                REQUIRE (link->first.lower() == 0);
                REQUIRE (link->first.upper() == 960);
                REQUIRE (link->second == s1.id());

                ++link;
                REQUIRE (link->first.lower() == 960);
                REQUIRE (link->first.upper() == 1920);
                REQUIRE (link->second == s2.id());

                std::ostringstream oss;
                printScreenLinks (oss, screens, links, 0);
                printScreenLinks (oss, screens, links, 1);
                printScreenLinks (oss, screens, links, 2);
                auto expected =
                    "Andrews-PC:\n"
                        "\tdown(0,50) = Andrews-Cray2(0,50)\n"
                        "\tdown(50,100) = Andrews-iMac(0,50)\n"
                    "Andrews-iMac:\n"
                        "\tup(0,50) = Andrews-PC(50,100)\n"
                        "\tdown(0,50) = Andrews-Cray2(50,100)\n"
                    "Andrews-Cray2:\n"
                        "\tup(0,50) = Andrews-PC(0,50)\n"
                        "\tup(50,100) = Andrews-iMac(0,50)\n";
                REQUIRE (expected == oss.str());
            }
        }
    };
}
