#include <synergy/common/ScreenSnapshot.h>
#include <synergy/service/json.hpp>

Screen
Screen::fromJsonSnapshot (std::string const& json) {
    Screen screen;
    Snapshot snapshot;
    from_json (snapshot, json);

    /* TODO: check and load the snapshot */
    return screen;
}
