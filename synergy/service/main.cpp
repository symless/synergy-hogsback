#include "ServiceController.h"
#include "synergy/common/RpcManager.h"
#include "synergy/common/WampRouter.h"
#include "synergy/common/WampServer.h"
#include "synergy/common/DirectoryManager.h"
#include <synergy/service/ProcessManager.h>

#include <iostream>

int
main (int argc, char* argv[]) {
    ServiceController serviceontroller;
    serviceontroller.parseArg(argc, argv);
    return serviceontroller.run();

    boost::asio::io_service mainIoService;

    ProcessManager processManager(mainIoService);
    processManager.onOutput.connect([](std::string line) {
        std::cout << line << std::endl;
    });

    RpcManager rpcManager(mainIoService);
    rpcManager.initRouterAndServer();
    rpcManager.startRouter();
    rpcManager.getServer()->startCore.connect([&processManager] (std::vector<std::string> cmd) { processManager.start(cmd); });

    //processManager.start ({DirectoryManager::instance()->installedDir().append("/synergyc"), "-f", "--debug", "DEBUG2", "192.168.3.30"});

    boost::asio::io_service::work work(mainIoService);
    mainIoService.run();

    return 0;
};
