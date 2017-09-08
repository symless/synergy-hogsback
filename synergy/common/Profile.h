#include <synergy/common/Screen.h>
#include <cstdint>
#include <string>
#include <vector>
#include <boost/signals2.hpp>

class Profile final {
public:
    template <typename... Args>
    using signal = boost::signals2::signal<Args...>;

    explicit Profile (int64_t id);
    static Profile fromJSONSnapshot (std::string const&);
    void merge (Profile);

private:
    Profile() = default;
    int64_t m_id;
    int64_t m_version;
    int64_t m_server;
    std::string m_name;
    std::vector<Screen> m_screens;

public:
    signal<void ()> layoutChanged;
    signal<void (std::vector<Screen> added, std::vector<Screen> removed)>
        screenSetChanged;
};
