#pragma once

#include <synergy/common/Profile.h>
#include <synergy/common/Screen.h>

#include <vector>
#include <boost/signals2.hpp>

class ProfileConfig final
{
public:
    static ProfileConfig fromJSONSnapshot (std::string const&json);

    void apply (ProfileConfig const& src);
    bool compare (ProfileConfig const& src);
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

//private:
//    ProfileConfig() = default;

    std::vector<Screen> screens() const;

private:
    Profile m_profile;
    std::vector<Screen> m_screens;
};
