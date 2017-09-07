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
    //void update (Snapshot const&);
private:
    int64_t m_id;
    int64_t m_version;
    std::string m_name;
    std::vector<Screen> m_screens;

public:
    signal<void ()> layoutChanged;
    signal<void (std::vector<Screen> added, std::vector<Screen> removed)>
        screenSetChanged;
};

