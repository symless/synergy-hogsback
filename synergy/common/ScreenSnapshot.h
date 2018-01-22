#pragma once

#include <synergy/service/json.hpp>

#include <string>

// NOTE: be aware of the padding
struct ScreenSnapshot {
    int        id;
    std::string     name;
    int        x_pos;
    int        y_pos;
    bool            active;
    std::string     status;
    std::string     ipList;
    uint64_t        version;
    uint32_t        error_code;
    std::string     error_message;
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
    (uint64_t,   version)
    (uint32_t,      error_code)
    (std::string,   error_message)
)
