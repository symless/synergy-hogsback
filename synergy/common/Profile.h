#pragma once
#include <synergy/common/Screen.h>
#include <cstdint>
#include <string>
#include <vector>
#include <boost/signals2.hpp>

class Profile final {
public:
    static Profile fromJSONSnapshot (std::string const&);

    explicit Profile(int64_t id) noexcept;

    void apply (Profile const& src);
    bool compare (Profile const& src);
    void clone (Profile const& src);

    int64_t id() const;
    void setId(const int64_t &id);
    int64_t version() const;
    void setVersion(const int64_t &version);
    int64_t server() const;
    void setServer(const int64_t &server);
    std::string name() const;
    const std::vector<Screen> &screens() const;

    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    signal<void ()> modified;

    signal<void (int64_t)> idChanged;
    signal<void (std::string)> nameChanged;
    signal<void (int64_t)> serverChanged;

    signal<void (int64_t)> screenNameChanged;
    signal<void (int64_t)> screenPositionChanged;
    signal<void (int64_t)> screenStatusChanged;

    signal<void (std::vector<Screen> added, std::vector<Screen> removed)>
        screenSetChanged;

private:
    Profile() = default;
    int64_t m_id;
    int64_t m_version;
    int64_t m_server;
    std::string m_name;
    std::vector<Screen> m_screens;
};
