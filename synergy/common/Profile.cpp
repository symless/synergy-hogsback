#include <synergy/common/Profile.h>

#include <synergy/common/ProfileSnapshot.h>

#include <synergy/service/json.hpp>

Profile
Profile::fromJSONSnapshot (std::string const& json) {
    Profile profile;
    ProfileSnapshot snapshot;
    try {
    from_json (snapshot, json);
}
    catch (std::exception& e) {
        std::string test = e.what();
    }

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

Profile::Profile(int64_t id) noexcept :
    m_id(id)
{

}

void Profile::apply (Profile const& src)
{
    if (compare(src)) {
        clone(src);
    }
}

bool Profile::compare(Profile const& src)
{
    bool different = false;

    if (src.version() <= m_version) {
        return different;
    }

    if (src.id() != m_id) {
        idChanged(src.id());
        different = true;
    }

    if (src.name() != m_name) {
        nameChanged(src.name());
        different = true;
    }

    if (src.server() != m_server) {
        serverChanged(src.server());
        different = true;
    }

    std::vector<Screen> added;
    for (const auto& srcScreen: src.screens()) {
        bool found = false;

        for (auto& screen : m_screens) {
            if (screen.id() == srcScreen.id()) {
                found = true;

                if (screen.name() != srcScreen.name()) {
                    screenNameChanged(srcScreen.id());
                    different = true;
                }

                if (screen.active() != srcScreen.active()) {
                    screenStatusChanged(srcScreen.id());
                    different = true;
                }

                if (screen.x() != srcScreen.x() || screen.y() != srcScreen.y()) {
                    screenPositionChanged(srcScreen.id());
                    different = true;
                }

                if (screen.status() != srcScreen.status()) {
                    screenStatusChanged(srcScreen.id());
                    different = true;
                }

                if (screen.successfulTestIp() != srcScreen.successfulTestIp() ||
                    screen.failedTestIp() != srcScreen.failedTestIp()) {
                    screenTestResultChanged(srcScreen.id(), srcScreen.successfulTestIp(), srcScreen.failedTestIp());
                    different = true;
                }
            }
        }

        if (!found) {
            added.push_back(srcScreen);
        }
    }

    std::vector<Screen> removed;
    for (const auto& screen: m_screens) {
        bool found = false;

        for (auto& srcScreen : src.screens()) {
            if (screen.id() == srcScreen.id()) {
                found = true;
            }
        }

        if (!found) {
            removed.push_back(screen);
        }
    }

    if (!added.empty() || !removed.empty()) {
        screenSetChanged(added, removed);
        different = true;
    }

    return different;
}

void Profile::clone (Profile const& src)
{
    m_id = src.id();
    m_version = src.version();
    m_server = src.server();
    m_name = src.name();
    m_screens = src.screens();
}

int64_t Profile::id() const
{
    return m_id;
}

void Profile::setId(const int64_t &id)
{
    if (id != m_id) {
        modified();
    }

    m_id = id;
}

int64_t Profile::version() const
{
    return m_version;
}

void Profile::setVersion(const int64_t &version)
{
    if (version != m_version) {
        modified();
    }

    m_version = version;
}

int64_t Profile::server() const
{
    return m_server;
}

void Profile::setServer(const int64_t &server)
{
    if (server != m_server) {
        modified();
    }

    m_server = server;
}

std::string Profile::name() const
{
    return m_name;
}

const std::vector<Screen>& Profile::screens() const
{
    return m_screens;
}
