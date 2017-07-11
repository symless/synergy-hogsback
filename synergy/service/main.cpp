#include "synergy/common/WampServer.h"
#include "synergy/common/WampRouter.h"
#include "synergy/common/DirectoryManager.h"
#include <synergy/service/ProcessManager.h>
#include <iostream>

const char* const kLocalIpAddress = "127.0.0.1";
const int kWampDefaultPort = 24888;
int
main (int argc, char* argv[]) {
    ProcessManager processManager;

    processManager.onOutput.connect([](std::string line) {
        std::cout << line << std::endl;
    });

    WampRouter wampRouter(kLocalIpAddress, kWampDefaultPort);

    WampServer wampServer;
    wampRouter.ready.connect([&wampServer](bool ready) {
        if (ready) {
            wampServer.start(kLocalIpAddress, kWampDefaultPort);
        }
        else {
            throw;
        }
    });

    processManager.start ({DirectoryManager::instance()->installedDir().append("/synergyc"), "-f", "--debug", "DEBUG2", "192.168.3.30"});
    processManager.run();

    return 0;
};
