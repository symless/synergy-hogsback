#include <synergy/common/ConfigGen.h>

using boost::icl::interval;
using boost::icl::interval_map;

enum class Direction : int { Up = 0, Right = 1, Down = 2, Left = 3 };
static char const* const directionNames[] = {"up", "right", "down", "left"};

static void
removeSource (Screen const& source, std::vector<ScreenLinkMap>& targets) {
    /* Remove the source from the target list (screens can't be linked to
     * themselves) */
    targets.erase (
        std::remove_if (begin (targets), end(targets), [&source](auto& p) {
            Screen const& target = p;
            return (&target == &source);
        }),
        end(targets)
    );
}

static void
removeLeft (Screen const& source, std::vector<ScreenLinkMap>& targets) {
    /* Remove potential targets to the left of source */
    targets.erase (
        std::remove_if (begin (targets), end(targets), [&source](auto& target) {
            return (target->x() + target->width()) <= source.x();
        }),
        end(targets)
    );
}

static void
removeRight (Screen const& source, std::vector<ScreenLinkMap>& targets) {
    /* Remove potential targets to the right of source */
    targets.erase (
        std::remove_if (begin (targets), end(targets), [&source](auto& target) {
            return target->x() >= (source.x() + source.width());
        }),
        end(targets)
    );
}

static void
removeAbove (Screen const& source, std::vector<ScreenLinkMap>& targets) {
    /* Remove potential targets above the source */
    targets.erase (
        std::remove_if (begin (targets), end(targets), [&source](auto& target) {
            return (target->y() + target->height()) <= source.y();
        }),
        end(targets)
    );
}

static void
removeBelow (Screen const& source, std::vector<ScreenLinkMap>& targets) {
    /* Remove potential targets below the source */
    targets.erase (
        std::remove_if (begin (targets), end(targets), [&source](auto& target) {
            return target->y() >= (source.y() + source.height());
        }),
        end(targets)
    );
}

static void
linkLeft (ScreenLinkMap const& source, std::vector<ScreenLinkMap> targets) {
    removeSource (source, targets);
    removeRight (source, targets);
    removeAbove (source, targets);
    removeBelow (source, targets);

    /* Sort targets in to ascending order by x-coordinate (so we link up the
     * left-most screens first, working right back toward the source).
     */
    std::sort (begin(targets), end(targets),
               [](auto t1, auto t2) { return t1->x() < t2->x(); });

    for (auto& target : targets) {
        auto i = interval<int64_t>::closed (
            std::max (source->y(), target->y()),
            std::min (source->y() + source->height(), target->y() + target->height())
        );
        source.edges()[int(Direction::Left)].set (make_pair (i, target->id()));
    }
}

static void
linkRight (ScreenLinkMap& source, std::vector<ScreenLinkMap> targets) {
    removeSource (source, targets);
    removeLeft (source, targets);
    removeAbove (source, targets);
    removeBelow (source, targets);

    /* Sort targets in to descending order by x-coordinate (so we link up the
     * right-most screens first, working left back toward the source).
     */
    std::sort (begin(targets), end(targets),
               [](auto t1, auto t2) { return t1->x() > t2->x(); });

    for (auto& target : targets) {
        auto i = interval<int64_t>::closed (
            std::max (source->y(), target->y()),
            std::min (source->y() + source->height(),
                        target->y() + target->height())
        );
        source.edges()[int(Direction::Right)].set (make_pair (i, target->id()));
    }
}

static void
linkUp (ScreenLinkMap& source, std::vector<ScreenLinkMap> targets) {
    removeSource (source, targets);
    removeLeft (source, targets);
    removeRight (source, targets);
    removeBelow (source, targets);

    /* Sort targets in to ascending order by y-coordinate (so we link up the
     * top-most screens first, working down back toward the source).
     */
    std::sort (begin(targets), end(targets),
               [](auto t1, auto t2) { return t1->y() < t2->y(); });

    for (auto& target : targets) {
        auto i = interval<int64_t>::closed (
            std::max (source->x(), target->x()),
            std::min (source->x() + source->width(),
                      target->x() + target->width())
        );
        source.edges()[int(Direction::Up)].set (make_pair (i, target->id()));
    }
}

static void
linkDown (ScreenLinkMap& source, std::vector<ScreenLinkMap> targets) {
    removeSource (source, targets);
    removeLeft (source, targets);
    removeRight (source, targets);
    removeAbove (source, targets);

    /* Sort targets in to descending order by y-coordinate (so we link up the
     * bottom-most screens first, working up back toward the source).
     */
    std::sort (begin(targets), end(targets),
               [](auto t1, auto t2) { return t1->y() > t2->y(); });

    for (auto& target : targets) {
        auto i = interval<int64_t>::closed (
            std::max (source->x(), target->x()),
            std::min (source->x() + source->width(),
                      target->x() + target->width())
        );
        source.edges()[int(Direction::Down)].set (make_pair (i, target->id()));
    }
}

std::vector<ScreenLinks>
linkScreens (std::vector<Screen> const& screens) {
    /* Create all the links */
    std::vector<ScreenLinks> links;
    links.resize (screens.size());

    /* SOA -> AOS */
    std::vector<ScreenLinkMap> targets;
    targets.reserve (screens.size());

    for (auto t = 0; t < screens.size(); ++t) {
        targets.emplace_back (&screens[t], &links[t]);
    }

    for (auto& target : targets) {
        linkUp (target, targets);
        linkRight (target, targets);
        linkDown (target, targets);
        linkLeft (target, targets);
    }

    return links;
}

void
printScreenLinks (std::ostream& os, std::vector<Screen> const& screens,
                  std::vector<ScreenLinks>& links, int const i) {
    assert (screens.size() == links.size());

    /* SOA -> AOS */
    std::vector<ScreenLinkMap> targets;
    targets.reserve (screens.size());

    for (auto t = 0; t < screens.size(); ++t) {
        targets.emplace_back (&screens[t], &links[t]);
    }

    return printScreenLinks (os, ScreenLinkMap(&screens.at(i), &links.at(i)),
                             targets);
}

void
printScreenLinks (std::ostream& os, ScreenLinkMap const& screen,
                  std::vector<ScreenLinkMap> const& targets) {
    os << screen->name() << ":\n";

    for (auto edge = 0; edge < 4; ++edge) {
        auto const& links = screen.edges()[edge];

        switch (edge & 1) {
            /* Top or bottom edge (vertical links) */
            case 0:
                for (auto link : links) {
                    auto upper   = link.first.upper ();
                    auto lower   = link.first.lower ();
                    double src_l = (lower - screen->x()) * 100.0 / screen->width();
                    double src_u = (upper - screen->x()) * 100.0 / screen->width();

                    auto dest = std::find_if (
                        begin (targets), end (targets), [&](auto& screen) {
                            return screen->id() == link.second;
                        });

                    assert (dest != end (targets));
                    auto& target = *dest;
                    double dst_l = (lower - target->x()) * 100.0 / target->width();
                    double dst_u = (upper - target->x()) * 100.0 / target->width();

                    os << "\t" << directionNames[edge]
                              << "(" << src_l << "," << src_u << ") = "
                              << target->name()
                              << "(" << dst_l << "," << dst_u << ")\n";
                }
                break;

            /* Left or right edge (horizontal links) */
            case 1:
                for (auto link : links) {
                    auto upper   = link.first.upper ();
                    auto lower   = link.first.lower ();
                    double src_l = (lower - screen->y()) * 100.0 / screen->height();
                    double src_u = (upper - screen->y()) * 100.0 / screen->height();

                    auto dest = std::find_if (
                        begin (targets), end (targets), [&](auto& screen) {
                            return screen->id() == link.second;
                        });

                    assert (dest != end (targets));
                    auto& target = *dest;
                    double dst_l = (lower - target->y()) * 100.0 / target->height();
                    double dst_u = (upper - target->y()) * 100.0 / target->height();

                    os << "\t" << directionNames[edge]
                              << "(" << src_l << "," << src_u << ") = "
                              << target->name()
                              << "(" << dst_l << "," << dst_u << ")\n";
                }
                break;
        }
    }
}

void
printConfig (std::ostream& os, std::vector<Screen> const& screens) {
    os << "section: options\n";
    os << "end\n\n";

    os << "section: screens\n";
    for (auto& screen: screens) {
        os << screen.name() << ":\n";
    }
    os << "end\n\n";

    os << "section: links\n";
    auto links = linkScreens(screens);
    for (int i = 0; i < screens.size(); ++i) {
        printScreenLinks (os, screens, links, i);
    }
    os << "end\n\n";
}
