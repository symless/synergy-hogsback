#pragma once

#include <synergy/common/Screen.h>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>

class Screen::Snapshot {
public:
    uint64_t        id;
    std::string     name;
    uint64_t        x_pos;
    uint64_t        y_pos;
    std::string     ipList;
    std::string     status;
    bool            active;
};

BOOST_FUSION_ADAPT_STRUCT(
    Screen::Snapshot,
    (uint64_t,      id)
    (std::string,   name)
    (uint64_t,      x_pos)
    (uint64_t,      y_pos)
    (std::string,   ipList)
    (std::string,   status)
    (bool,          active)
)
