#include <synergy/common/Profile.h>
#include <synergy/common/ProfileSnapshot.h>
#include <synergy/service/json.hpp>

Profile
Profile::fromJsonSnapshot (std::string const& json) {
    Profile profile;
    Snapshot snapshot;
    from_json (snapshot, json);

    profile.m_id = snapshot.id;
    profile.m_version = snapshot.configVersion;
    profile.m_server = snapshot.server;
    profile.m_name = snapshot.name;
    profile.m_screens.reserve (snapshot.screens.size());

    for (auto const& screenSnapshot: snapshot.screens) {
        Screen screen;
        screen.apply (screenSnapshot);
        profile.m_screens.push_back (std::move (screen));
    }

    return profile;
}
