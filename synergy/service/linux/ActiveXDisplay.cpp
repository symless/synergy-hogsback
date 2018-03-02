#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/optional.hpp>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>

namespace {

struct DisplayInfo {
    int useCount = 0;
    boost::optional<std::string> authority;
};

std::string
getActiveTTY() {
    std::string activeTTY;
    std::ifstream ifs ("/sys/class/tty/tty0/active");
    ifs.exceptions (std::ios::badbit | std::ios::failbit);
    std::getline (ifs, activeTTY);
    return activeTTY;
}

bool
isNumericString (std::string const& str, int* const out = 0) noexcept {
    try {
        auto val = std::stoi (str);
        if (out) {
          *out = val;
        }
        return true;
    } catch(...) {
    }
    return false;
}

auto
getProcessXDisplayVars (boost::filesystem::path const& path) {
    boost::optional<std::string> display;
    boost::optional<std::string> authority;

    std::ifstream ifs (path.string(), std::ios::in | std::ios::binary);
    std::string envline;
    std::vector<std::string> kv;

    while (std::getline (ifs, envline, '\0')) {
       boost::split (kv, envline, boost::is_any_of("="));
       if (kv.size() != 2) {
            continue;
       }
       if (kv[0] == "DISPLAY") {
            display = std::move(kv[1]);
       }
       else if (kv[0] == "XAUTHORITY") {
            authority = std::move(kv[1]);
       }
    }

    return std::make_pair (std::move(display), std::move(authority));
}

auto
getXDisplaysOnTTY (std::string const& tty) {
    std::map<std::string, DisplayInfo> displayInfo;
    auto ttyFile = boost::filesystem::path ("/dev") / tty;

    struct stat ttyStat;
    if (0 != ::stat (ttyFile.string().data(), &ttyStat)) {
        return displayInfo;
    }

    boost::filesystem::directory_iterator it ("/proc");
    boost::filesystem::directory_iterator end;

    std::string statsLine;
    std::vector<std::string> stats;

    for (; it != end; ++it) {
        pid_t pid;
        if (!isNumericString (it->path().filename().string(), &pid)) {
           continue;
        }

        std::ifstream statsFile ((it->path() / "stat").string());
        statsFile.exceptions (std::ios::badbit | std::ios::failbit);

        std::getline (statsFile, statsLine);
        boost::split (stats, statsLine, boost::is_any_of(" "),
                     boost::algorithm::token_compress_on);

        if (stats.size() < 7) {
           continue;
        }

        int tty = 0;
        if (!isNumericString (stats[6], &tty) || (tty == 0)) {
            continue;
        }

        if (tty != ttyStat.st_rdev) {
            continue;
        }

        /*char exe[PATH_MAX] = {0};
        if (0 >= ::readlink ((it->path() / "exe").string().data(), exe, sizeof(exe))) {
            continue;
        }*/

        auto xvars = getProcessXDisplayVars (it->path() / "environ");
        if (!xvars.first) {
           continue;
        }

        auto& info = displayInfo[*xvars.first];
        info.useCount++;
        if (!info.authority) {
            info.authority = std::move(xvars.second);
        }
    }

    return displayInfo;
}

} // namespace

bool
getActiveTTYXDisplay
(boost::optional<std::string>& display) noexcept try {
    auto displays = getXDisplaysOnTTY (getActiveTTY());

    auto mostUsedDisplay = std::max_element (begin(displays), end(displays),
                            [](auto& a, auto& b) {
        return a.second.useCount < b.second.useCount;
    });

    if (mostUsedDisplay != end(displays)) {
        display = mostUsedDisplay->first;
        return true;
    }

    return false;
}
catch (...) {
    return false;
}

