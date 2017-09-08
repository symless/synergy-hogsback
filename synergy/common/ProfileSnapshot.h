#pragma once
#include <synergy/common/Profile.h>
#include <synergy/common/ScreenSnapshot.h>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>

class Profile::Snapshot {
public:
    uint64_t    id;
    uint64_t    configVersion;
    uint64_t    server;
    std::string name;
    std::vector<Screen::Snapshot> screens;
};

BOOST_FUSION_ADAPT_STRUCT(
    Profile::Snapshot,
    (uint64_t, id)
    (uint64_t, configVersion)
    (uint64_t, server)
    (std::string, name)
    (std::vector<Screen::Snapshot>, screens)
);
