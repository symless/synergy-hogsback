#include <cstdint>
#include <array>
#include <vector>
#include <boost/icl/interval_map.hpp>

using ScreenID = int64_t;
using ScreenLinkMap = boost::icl::interval_map<int64_t, ScreenID>;
enum class ScreenLinkDirection : int { Up = 0, Right = 1, Down = 2, Left = 3 };

struct Screen {
    ScreenID id = 0;
    std::string name;
    int64_t x      = 0;
    int64_t y      = 0;
    int64_t width  = 0;
    int64_t height = 0;
    std::array<ScreenLinkMap, 4> sides;
    // TODO: add (weak) pointer back to Config
};

struct Config {
    // TODO: add getScreen(id) function to access a screen
    std::vector<Screen> screens;
};
