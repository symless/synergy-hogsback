#include <synergy/tray/Log.h>

static std::shared_ptr<spdlog::logger>
initTrayLog() {
    return nullptr;
}

std::shared_ptr<spdlog::logger> const&
trayLog() {
    static auto log = initTrayLog();
    return log;
}
