#pragma once
#include <synergy/common/Screen.h>
#include <cstdint>
#include <string>
#include <vector>
#include <boost/signals2.hpp>

class Profile final {
public:
    class Snapshot;
    static Profile fromJsonSnapshot (std::string const&);

    explicit Profile (int64_t id);
    void apply (Snapshot const&);
    const std::vector<Screen>& getScreens() const;

private:
    Profile() = default;
    int64_t m_id;
    int64_t m_version;
    int64_t m_server;
    std::string m_name;
    std::vector<Screen> m_screens;

public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void ()> layoutChanged;
    signal<void (std::vector<Screen> added, std::vector<Screen> removed)>
        screenSetChanged;
};
