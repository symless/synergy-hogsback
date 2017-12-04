#pragma once

#include <synergy/common/Profile.h>
#include <synergy/common/Screen.h>

#include <vector>
#include <boost/signals2.hpp>

class ProfileConfig final
{
public:
    static ProfileConfig fromJsonSnapshot (std::string const&json);

    void apply (ProfileConfig const& src);
    bool compare (ProfileConfig const& target);
    void clone (ProfileConfig const& src);

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void ()> modified;

    signal<void (int64_t)> profileIdChanged;
    signal<void (std::string)> profileNameChanged;
    signal<void (int64_t)> profileServerChanged;

    signal<void (int64_t)> screenNameChanged;
    signal<void (int64_t)> screenPositionChanged;
    signal<void (int64_t)> screenStatusChanged;
    signal<void (int64_t, std::string, std::string)> screenTestResultChanged;

    signal<void (std::vector<Screen> added, std::vector<Screen> removed)>
        screenSetChanged;
    signal<void (Screen screen)> screenOnline;

    std::vector<Screen> screens() const;
    void updateScreenTestResult(int screenId, std::string successfulIp, std::string failedIp);
    Screen& getScreen(int screenId);
    bool hasServer() const { return m_profile.server() != -1; } // TODO: remove hack
    int profileId() const { return m_profile.id(); }

private:
    Profile m_profile;
    std::vector<Screen> m_screens;
};
