#include "synergy/common/DirectoryManager.h"
#include <synergy/service/ProcessManager.h>
#include <iostream>

int
main (int argc, char* argv[]) {
    ProcessManager processManager;

    processManager.onOutput.connect([](std::string line) {
        std::cout << line << std::endl;
    });

    processManager.start ({DirectoryManager::instance()->installedDir().append("/synergyc.exe"), "-f", "--debug", "DEBUG2", "192.168.3.30"});
    processManager.run();

    return 0;
};
