#include <synergy/common/Profile.h>
#include <synergy/service/json.hpp>

DEFINE_JSON(
    Snapshot,
    (uint64_t, id)
    (std::string, name)
    (uint64_t, configVersion)
    (uint64_t, server)
);

Profile::Profile (int64_t const id): m_id (id) {

}

Profile
Profile::fromJSONSnapshot (std::string const& json) {
    Profile profile;
    Snapshot snapshot;
    from_json (snapshot, json);

    profile.m_id = snapshot.id;
    profile.m_version = snapshot.configVersion;
    profile.m_name = snapshot.name;
    profile.m_server = snapshot.server;

    return profile;
}
