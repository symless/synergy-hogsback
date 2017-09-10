#pragma once

#include <synergy/service/json.hpp>

#include <string>

struct ProfileSnapshot {
    int    id;
    std::string name;
    int    server;
    int    configVersion;
};

BOOST_FUSION_ADAPT_STRUCT(
    ProfileSnapshot,
    (int, id)
    (std::string, name)
    (int, server)
    (int, configVersion)
);
