#include "UserConfig.h"
#include "GlobalConfig.h"
#include "SessionManager.h"
#include "ProcessManager.h"

#include <thread>
#include <boost/process.hpp>

void
initLogging() {}

void
initPlatformService() {}

void ioLoop()
{
//    startCore();

//    while (1) {
//        // event loop
//        if (thereIsSwitch()) {
//            startCore();
//        }
//    }
}
void
loadConfig()
{
    GlobalConfig gConfig;
    UserConfig uConfig;
    //auto globalConfig = loadGlobalConfiguration();
    //auto userConfig = loadUserConfiguration(globalConfig(), currentUserId());
}

int
main (int argc, char *argv[])
{
    initLogging();
    initPlatformService();
    loadConfig();

    SessionManager sessionManager;
    ProcessManager processManager;

    // connect all signals
    // user changed -> updateGlobalConfig -> restart core

    sessionManager.sessionChanged.connect([](int x) {});

    std::thread ioThread(ioLoop);

    return 0;
};
