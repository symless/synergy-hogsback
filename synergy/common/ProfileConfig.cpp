#include "ProfileConfig.h"

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
        clone(src);
    }
}

bool ProfileConfig::compare(ProfileConfig const& src)
{
    bool different = false;

    if (src.m_profile.m_version < m_profile.m_version) {
        return different;
    }

    if (src.m_profile.m_id != m_profile.m_id) {
        profileIdChanged(src.m_profile.m_id);
        different = true;
    }

    if (src.m_profile.m_name != m_profile.m_name) {
        profileNameChanged(src.m_profile.m_name);
        different = true;
    }

    if (src.m_profile.m_server != m_profile.m_server) {
        profileServerChanged(src.m_profile.m_server);
        different = true;
    }

    std::vector<Screen> added;
    for (const auto& srcScreen: src.m_screens) {
        bool found = false;

        for (auto& screen : m_screens) {
            if (screen.m_id == srcScreen.m_id) {
                found = true;

                if (screen.m_name != srcScreen.m_name) {
                    screenNameChanged(srcScreen.m_id);
                    different = true;
                }

                if (screen.m_active != srcScreen.m_active) {
                    screenStatusChanged(srcScreen.m_id);
                    different = true;
                }

                if (screen.m_x != srcScreen.m_x || screen.m_y != srcScreen.m_y) {
                    screenPositionChanged(srcScreen.m_id);
                    different = true;
                }

                if (screen.m_status != srcScreen.m_status) {
                    screenStatusChanged(srcScreen.m_id);
                    different = true;
                }

                if (screen.m_successfulTestIp != srcScreen.m_successfulTestIp ||
                    screen.m_failedTestIp != srcScreen.m_failedTestIp) {
                    screenTestResultChanged(srcScreen.m_id, srcScreen.m_successfulTestIp, srcScreen.m_failedTestIp);
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

        for (auto& srcScreen : src.m_screens) {
            if (screen.m_id == srcScreen.m_id) {
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

void ProfileConfig::claimServer(int serverId)
{
    m_profile.m_server = serverId;

    modified();
}
