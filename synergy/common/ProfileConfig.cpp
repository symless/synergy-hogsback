#include "ProfileConfig.h"

#include <synergy/service/ServiceLogs.h>
#include <synergy/common/ProfileConfigSnapshot.h>"

#include <synergy/service/json.hpp>

ProfileConfig
ProfileConfig::fromJsonSnapshot(const std::string& json)
{
    ProfileConfigSnapshot snapshot;
    from_json (snapshot, json);

    ProfileConfig profileConfig;
    profileConfig.m_profile.m_id = snapshot.profile.id;
    profileConfig.m_profile.m_version = snapshot.profile.configVersion;
    profileConfig.m_profile.m_server = snapshot.profile.server;
    profileConfig.m_profile.m_name = snapshot.profile.name;

    profileConfig.m_screens.reserve (snapshot.screens.size());
    for (auto const& screenSnapshot: snapshot.screens) {
        Screen screen;
        screen.apply (screenSnapshot);
        profileConfig.m_screens.push_back (std::move (screen));
    }

    return profileConfig;
}

void ProfileConfig::apply (ProfileConfig const& src)
{
    if (compare(src)) {
        serviceLog()->debug("profile changed, storing local copy");
        *this = src;
    }
    else {
        serviceLog()->debug("profile has not changed, no action needed");
    }
}

bool ProfileConfig::compare(ProfileConfig const& target)
{
    serviceLog()->debug(
        "comparing profiles, id={} this=v{} other=v{}",
        m_profile.id(), m_profile.m_version, target.m_profile.m_version);

    bool different = false;

    if (target.m_profile.m_version < m_profile.m_version) {
        serviceLog()->debug("profile version is older ({} < {}), abort compare",
            target.m_profile.m_version, m_profile.m_version);
        return different;
    }

    if (target.m_profile.m_id != m_profile.m_id) {
        serviceLog()->debug("profile id changed, {}->{}", m_profile.m_id, target.m_profile.m_id);
        profileIdChanged(target.m_profile.m_id);
        different = true;
    }

    if (target.m_profile.m_name != m_profile.m_name) {
        serviceLog()->debug("profile name changed, {}->{}", m_profile.m_name, target.m_profile.m_name);
        profileNameChanged(target.m_profile.m_name);
        different = true;
    }

    if (target.m_profile.m_server != m_profile.m_server) {
        serviceLog()->debug("profile server changed, {}->{}", m_profile.m_server, target.m_profile.m_server);
        profileServerChanged(target.m_profile.m_server);
        different = true;
    }

    std::vector<Screen> added;
    for (const auto& targetScreen: target.m_screens) {
        bool found = false;

        for (auto& screen : m_screens) {
            if (screen.id() == targetScreen.id()) {
                found = true;

                if (screen.version() > targetScreen.version()) {
                    serviceLog()->debug("screen version is older ({} < {}), skip screen {}",
                        targetScreen.version(), screen.version(), screen.id());
                    continue;
                }

                if (screen.name() != targetScreen.name()) {
                    serviceLog()->debug("profile screen name changed, screenId={} {}->{}",
                        screen.id(), screen.name(), targetScreen.name());
                    screenNameChanged(targetScreen.id());
                    different = true;
                }

                if (screen.active() != targetScreen.active()) {
                    serviceLog()->debug("profile screen active changed, screenId={} {}->{}",
                        screen.id(), screen.active(), targetScreen.active());

                    if (targetScreen.active()) {
                        screenOnline(targetScreen);
                    }
                    else {
                        screenOffline(targetScreen);
                    }

                    different = true;
                }

                if (screen.ipList() != targetScreen.ipList()) {
                    serviceLog()->debug("profile screen IP set changed, screenId={}, {}->{}",
                        screen.id(), screen.ipList(), targetScreen.ipList());
                    screenIPSetChanged(targetScreen);
                    different = true;
                }

                if (screen.x() != targetScreen.x() ||
                        screen.y() != targetScreen.y()) {
                    serviceLog()->debug("profile screen position changed, screenId={} {},{}->{},{}",
                        screen.id(), screen.x(), screen.y(), targetScreen.x(), targetScreen.y());
                    screenPositionChanged(targetScreen.id());
                    different = true;
                }

                if (screen.status() != targetScreen.status()) {
                    serviceLog()->debug("profile screen status changed, screenId={} {}->{}",
                        screen.id(), screenStatusToString(screen.status()),
                        screenStatusToString(targetScreen.status()));
                    screenStatusChanged(targetScreen.id());
                    different = true;
                }
            }
        }

        if (!found) {
            added.push_back(targetScreen);
        }
    }

    std::vector<Screen> removed;
    for (const auto& screen: m_screens) {
        bool found = false;

        for (auto& srcScreen : target.m_screens) {
            if (screen.id() == srcScreen.id()) {
                found = true;
            }
        }

        if (!found) {
            removed.push_back(screen);
        }
    }

    if (!added.empty() || !removed.empty()) {
        serviceLog()->debug("profile screen set changed, added={} removed={}",
            added.size(), removed.size());
        screenSetChanged(added, removed);

        for (auto& addedScreen: added) {
            if (addedScreen.active()) {
                screenOnline (addedScreen);
            }
        }

        different = true;
    }

    return different;
}

void ProfileConfig::clear()
{
    m_profile = Profile();
    m_screens.clear();
}

ProfileConfig::ProfileConfig (ProfileConfig const& src):
    m_profile (src.m_profile), m_screens (src.m_screens) {
}

ProfileConfig&
ProfileConfig::operator= (ProfileConfig const& src) {
    m_profile = src.m_profile;
    m_screens = src.m_screens;
    return *this;
}

const std::vector<Screen> & ProfileConfig::screens() const
{
    return m_screens;
}

Screen& ProfileConfig::getScreen(const int screenId)
{
    for (auto& screen : m_screens) {
        if (screen.id() == screenId) {
            return screen;
        }
    }

    throw std::runtime_error("Can't find screen with ID: " + std::to_string(screenId));
}

void ProfileConfig::claimServer(int64_t serverId)
{
    m_profile.setServer(serverId);
    m_profile.touch();
    modified();
}
