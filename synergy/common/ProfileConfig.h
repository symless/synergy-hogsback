#pragma once

#include <synergy/common/Profile.h>
#include <synergy/common/Screen.h>

#include <vector>
#include <boost/signals2.hpp>

class ProfileConfig final
{
public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    static ProfileConfig fromJsonSnapshot (std::string const&json);

    ProfileConfig () = default;
    ProfileConfig (ProfileConfig const& src);
    ProfileConfig& operator= (ProfileConfig const& src);

    void apply (ProfileConfig const& src);
    bool compare (ProfileConfig const& target);

    signal<void ()> modified;

    signal<void (int64_t)> profileIdChanged;
    signal<void (std::string)> profileNameChanged;
    signal<void (int64_t)> profileServerChanged;

    signal<void (int64_t)> screenNameChanged;
    signal<void (Screen const&)> screenIPSetChanged;
    signal<void (int64_t)> screenPositionChanged;
    signal<void (int64_t)> screenStatusChanged;

    signal<void (std::vector<Screen> const& added,
                 std::vector<Screen> const& removed)> screenSetChanged;
    signal<void (Screen const& screen)> screenOnline;
    signal<void (Screen const& screen)> screenOffline;

    const std::vector<Screen>& screens() const;
    Screen& getScreen(const int screenId);
    Screen& getScreen(const std::string& screenName);
    int profileId() const { return m_profile.id(); }
    int64_t profileVersion() const { return m_profile.version(); }
    void claimServer(int64_t serverId);

private:
    Profile m_profile;
    std::vector<Screen> m_screens;
};
