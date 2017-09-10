#pragma once

#include <synergy/service/json.hpp>

#include <string>

struct ScreenSnapshot {
    int        id;
    std::string     name;
    int        x_pos;
    int        y_pos;
    bool            active;
    std::string     status;
    std::string     ipList;
};

BOOST_FUSION_ADAPT_STRUCT(
    ScreenSnapshot,
    (int,      id)
    (std::string,   name)
    (int,      x_pos)
    (int,      y_pos)
    (bool,          active)
    (std::string,   status)
    (std::string,   ipList)
)
