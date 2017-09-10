#pragma once

#include <synergy/service/json.hpp>

#include <string>

// NOTE: be aware of the padding
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
