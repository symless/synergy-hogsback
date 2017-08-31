#include <boost/icl/interval_map.hpp>
#include <cassert>
#include <cstdint>
#include <vector>
#include <ostream>
#include <array>
#include <synergy/service/Screen.h>

using boost::icl::interval;
using boost::icl::interval_map;

static char const* const directionNames[] = {"up", "right", "down", "left"};

static void
removeSelf (Screen& screen, std::vector<Screen*>& targets) {
    /* Remove source from the target list */
    targets.erase (
        std::remove_if (begin (targets), end(targets), [&screen](auto& target) {
            return target == &screen;
        }),
        end(targets)
    );
}

static void
removeLeft (Screen& screen, std::vector<Screen*>& targets) {
    /* Remove targets to the left of source */
    targets.erase (
        std::remove_if (begin (targets), end(targets), [&screen](auto& target) {
             return (target->x + target->width) <= screen.x;
        }),
        end(targets)
    );
}

static void
removeRight (Screen& screen, std::vector<Screen*>& targets) {
    /* Remove targets to the right of source */
    targets.erase (
        std::remove_if (begin (targets), end(targets), [&screen](auto& target) {
             return target->x >= (screen.x + screen.width);
        }),
        end(targets)
    );
}

static void
removeAbove (Screen& screen, std::vector<Screen*>& targets) {
    /* Remove targets above the source */
    targets.erase (
        std::remove_if (begin (targets), end(targets), [&screen](auto& target) {
             return (target->y + target->height) <= screen.y;
        }),
        end(targets)
    );
}

static void
removeBelow (Screen& screen, std::vector<Screen*>& targets) {
    /* Remove targets below the source */
    targets.erase (
        std::remove_if (begin (targets), end(targets), [&screen](auto& target) {
             return target->y >= (screen.y + screen.height);
        }),
        end(targets)
    );
}

static void
linkHorizontally (Screen& screen, std::vector<Screen*> targets) {
    removeSelf (screen, targets);
    removeLeft (screen, targets);
    removeAbove (screen, targets);
    removeBelow (screen, targets);

    std::sort (begin(targets), end(targets),
               [](auto t1, auto t2) { return t1->x > t2->x; });

    auto const Right = ScreenLinkDirection::Right;
    auto const Left = ScreenLinkDirection::Left;

    for (auto target : targets) {
        auto i = interval<int64_t>::closed (
            std::max (screen.y, target->y),
            std::min (screen.y + screen.height, target->y + target->height)
        );

        screen.sides[int(Right)].set (make_pair (i, target->id));
        target->sides[int(Left)].set (make_pair (i, screen.id));
    }
}

static void
linkVertically (Screen& screen, std::vector<Screen*> targets) {
    removeSelf (screen, targets);
    removeLeft (screen, targets);
    removeRight (screen, targets);
    removeBelow (screen, targets);

    std::sort (begin(targets), end(targets),
               [](auto t1, auto t2) { return t1->y < t2->y; });

    auto const Up = ScreenLinkDirection::Right;
    auto const Down = ScreenLinkDirection::Left;

    for (auto target : targets) {
        auto i = interval<int64_t>::closed (
            std::max (screen.x, target->x),
            std::min (screen.x + screen.width, target->x + target->width)
        );

        screen.sides[int(Up)].set (make_pair (i, target->id));
        target->sides[int(Down)].set (make_pair (i, screen.id));
    }
}

// TODO: refactor to operator<< (std::ostream&, Screen const&)
void
printScreenLinks (std::ostream& os, Screen const& screen,
                  std::vector<Screen> const& targets) {
    os << screen.name << ":" << std::endl;

    for (auto side = 0; side < 4; ++side) {
        auto const& links = screen.sides[side];

        switch (side & 1) {
            /* Top or bottom edge (vertical links) */
            case 0:
                for (auto link : links) {
                    auto upper   = link.first.upper ();
                    auto lower   = link.first.lower ();
                    double src_l = (lower - screen.x) * 100.0 / screen.width;
                    double src_u = (upper - screen.x) * 100.0 / screen.width;

                    auto dest = std::find_if (
                        begin (targets), end (targets), [&](auto& s) {
                            return s.id == link.second;
                        });

                    assert (dest != end (targets));
                    double dst_l = (lower - dest->x) * 100.0 / dest->width;
                    double dst_u = (upper - dest->x) * 100.0 / dest->width;

                    os << "\t" << directionNames[side]
                              << "(" << src_l << "," << src_u << ") = "
                              << dest->name
                              << "(" << dst_l << "," << dst_u << ")"
                              << std::endl;
                }
                break;

            /* Left or right edge (horizontal links) */
            case 1:
                for (auto link : links) {
                    auto upper   = link.first.upper ();
                    auto lower   = link.first.lower ();
                    double src_l = (lower - screen.y) * 100.0 / screen.height;
                    double src_u = (upper - screen.y) * 100.0 / screen.height;

                    auto dest = std::find_if (
                        begin (targets), end (targets), [&](auto& s) {
                            return s.id == link.second;
                        });

                    assert (dest != end (targets));
                    double dst_l = (lower - dest->y) * 100.0 / dest->height;
                    double dst_u = (upper - dest->y) * 100.0 / dest->height;

                    os << "\t" << directionNames[side]
                              << "(" << src_l << "," << src_u << ") = "
                              << dest->name
                              << "(" << dst_l << "," << dst_u << ")"
                              << std::endl;
                }
                break;
        }
    }
}

void
linkScreens (std::vector<Screen>& screens) {
    for (auto& screen: screens) {
        screen.sides.fill(ScreenLinkMap());
    }

    /* Create a field of targets to filter on */
    std::vector<Screen*> targets;
    std::transform (begin(screens), end(screens), std::back_inserter(targets),
                    [](auto& screen) { return &screen; });

    for (auto& screen : screens) {
        linkVertically (screen, targets);
        linkHorizontally (screen, targets);
    }
}

#if 0
#include <iomanip>

int
main () {
    std::vector<Screen> screens {
        {1, "Andrews-PC", 0, 0, 1920, 1080,{}},
        {2, "Andrews-iMac", 1920, 400, 1920, 1080,{}},
        {3, "Andrews-Linux-box", -640, 1080, 2560, 1080,{}},
        {4, "Andrews-GamingRig", 3840, 0, 800, 2160,{}}
    };

    linkScreens (screens);

    std::cout << "\n====\n";
    std::cout << std::setprecision (4) << std::endl;

    for (auto& s : screens) {
        printScreenLinks (std::cout, s, screens);
    }
}
#endif
