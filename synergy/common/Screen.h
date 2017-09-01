#ifndef SYNERGY_COMMON_SCREEN_H
#define SYNERGY_COMMON_SCREEN_H

#include <cstdint>
#include <string>

using ScreenID = int64_t;

class Screen final {
public:
    ScreenID    id      = 0;
    std::string name;
    int64_t     x       = 0;
    int64_t     y       = 0;
    int64_t     width   = 0;
    int64_t     height  = 0;
};

#endif
