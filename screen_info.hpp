#ifndef SCREEN_INFO_HPP
#define SCREEN_INFO_HPP

#include <boost/algorithm/string/replace.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <ostream>

enum ScreenMode : char {
    SERVER = 'S', // Server looking for clients
    CLIENT = 'C'  // Client looking for server
};

struct ScreenInfo {
    std::string name;
    boost::asio::ip::address ip_address;
    boost::posix_time::ptime last_seen;
    std::uint32_t version      = 20000;
    std::uint16_t service_port = 24810;
    ScreenMode mode            = ScreenMode::SERVER;
};

inline void
print_version (std::ostream& os, std::uint32_t const version) {
    os << (version / (100 * 100)) << "." << ((version % (100 * 100)) / 100)
       << "." << (version % 100);
}

inline std::ostream&
operator<< (std::ostream& os, ScreenInfo const& screen) {
    os << "{";
    os.put (static_cast<char> (screen.mode));
    os << ",";
    {
        auto screen_name =
            boost::algorithm::replace_all_copy (screen.name, "\\", "\\\\");
        boost::algorithm::replace_all (screen_name, "\"", "\\\"");
        os << "\"" << screen_name << "\"";
    }
    os << "," << screen.ip_address << ":" << screen.service_port << ",";
    print_version (os, screen.version);
    os << "}";
    return os;
}

inline std::istream&
operator>> (std::istream& os, ScreenInfo const&) {
    return os;
}


#endif // SCREEN_INFO_HPP
