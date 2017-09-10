#pragma once

#include <synergy/service/json.hpp>

#include <synergy/common/ProfileSnapshot.h>
#include <synergy/common/ScreenSnapshot.h>
#include <vector>

struct ProfileConfigSnapshot {
    ProfileSnapshot profile;
    std::vector<ScreenSnapshot> screens;
};

BOOST_FUSION_ADAPT_STRUCT(
    ProfileConfigSnapshot,
    (ProfileSnapshot, profile)
    (std::vector<ScreenSnapshot>, screens)
);
