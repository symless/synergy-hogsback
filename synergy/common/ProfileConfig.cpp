#include "ProfileConfig.h"

#include <synergy/service/Logs.h>
#include <synergy/common/ProfileConfigSnapshot.h>"

#include <synergy/service/json.hpp>

ProfileConfig ProfileConfig::fromJSONSnapshot(const std::string& json)
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
        mainLog()->debug("profile changed, storing local copy");
        clone(src);
    }
    else {
        mainLog()->debug("profile has not changed, no action needed");
    }
}

bool ProfileConfig::compare(ProfileConfig const& target)
{
    mainLog()->debug(
        "comparing profiles, id={} this=v{} other=v{}",
        m_profile.id(), m_profile.m_version, target.m_profile.m_version);

    bool different = false;

    if (target.m_profile.m_version < m_profile.m_version) {
        mainLog()->debug("profile version is older ({} < {}), abort compare",
            m_profile.m_version, target.m_profile.m_version);
        return different;
    }

    if (target.m_profile.m_id != m_profile.m_id) {
        mainLog()->debug("profile id changed, {}->{}", m_profile.m_id, target.m_profile.m_id);
        profileIdChanged(target.m_profile.m_id);
        different = true;
    }

    if (target.m_profile.m_name != m_profile.m_name) {
        mainLog()->debug("profile name changed, {}->{}", m_profile.m_name, target.m_profile.m_name);
        profileNameChanged(target.m_profile.m_name);
        different = true;
    }

    if (target.m_profile.m_server != m_profile.m_server) {
        mainLog()->debug("profile server changed, {}->{}", m_profile.m_server, target.m_profile.m_server);
        profileServerChanged(target.m_profile.m_server);
        different = true;
    }

    std::vector<Screen> added;
    for (const auto& targetScreen: target.m_screens) {
        bool found = false;

        for (auto& screen : m_screens) {
            if (screen.m_id == targetScreen.m_id) {
                found = true;

                if (screen.m_name != targetScreen.m_name) {
                    mainLog()->debug("profile screen name changed, screenId={} {}->{}",
                        screen.m_id, screen.m_name, targetScreen.m_name);
                    screenNameChanged(targetScreen.m_id);
                    different = true;
                }

                if (screen.m_active != targetScreen.m_active) {
                    mainLog()->debug("profile screen active changed, screenId={} {}->{}",
                        screen.m_id, screen.m_active, targetScreen.m_active);

                    if (targetScreen.m_active) {
                        screenOnline(targetScreen);
                    }

                    different = true;
                }

                if (screen.m_x != targetScreen.m_x || screen.m_y != targetScreen.m_y) {
                    mainLog()->debug("profile screen position changed, screenId={} {},{}->{},{}",
                        screen.m_id, screen.m_x, screen.m_y, targetScreen.m_x, targetScreen.m_y);
                    screenPositionChanged(targetScreen.m_id);
                    different = true;
                }

                if (screen.m_status != targetScreen.m_status) {
                    mainLog()->debug("profile screen status changed, screenId={} {}->{}",
                        screen.m_id, screenStatusToString(screen.m_status),
                        screenStatusToString(targetScreen.m_status));
                    screenStatusChanged(targetScreen.m_id);
                    different = true;
                }

                if ((screen.m_successfulTestIp != targetScreen.m_successfulTestIp) ||
                    (screen.m_failedTestIp != targetScreen.m_failedTestIp)) {
                    mainLog()->debug("profile screen test result changed, screenId={} success=`{}`->`{}` failed=`{}`->`{}`",
                        screen.m_id, screen.m_successfulTestIp, targetScreen.m_successfulTestIp,
                        screen.m_failedTestIp, targetScreen.m_failedTestIp);
                    screenTestResultChanged(targetScreen.m_id, targetScreen.m_successfulTestIp, targetScreen.m_failedTestIp);
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
            if (screen.m_id == srcScreen.m_id) {
                found = true;
            }
        }

        if (!found) {
            removed.push_back(screen);
        }
    }

    if (!added.empty() || !removed.empty()) {
        mainLog()->debug("profile screen set changed, added={} removed={}",
            added.size(), removed.size());
        screenSetChanged(added, removed);
        different = true;
    }

    return different;
}

void ProfileConfig::clone (ProfileConfig const& src)
{
    m_profile.m_id = src.m_profile.m_id;
    m_profile.m_version = src.m_profile.m_version;
    m_profile.m_server = src.m_profile.m_server;
    m_profile.m_name = src.m_profile.m_name;

    m_screens = src.m_screens;
}

std::vector<Screen> ProfileConfig::screens() const
{
    return m_screens;
}

void ProfileConfig::updateScreenTestResult(int screenId, std::string successfulIp, std::string failedIp)
{
    auto& screen = getScreen(screenId);
    screen.m_successfulTestIp = successfulIp;
    screen.m_failedTestIp = failedIp;

    modified();
}

Screen& ProfileConfig::getScreen(int screenId)
{
    for (auto& screen : m_screens) {
        if (screen.id() == screenId) {
            return screen;
        }
    }

    throw std::runtime_error("Can't find screen with ID: " + std::to_string(screenId));
}

void ProfileConfig::forceConnectivityTest()
{
    /* HACK: when the config UI opens, it requests a profile snapshot,
     * this should trigger us to "poke" the profile and cause
     * the connectivity test to run */
    std::vector<Screen> empty;
    screenSetChanged(m_screens, empty);
}
