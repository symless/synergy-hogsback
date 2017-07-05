#include <synergy/service/ProcessManager.h>
#include <iostream>

int
main (int argc, char* argv[]) {
    ProcessManager processManager;

    processManager.onOutput.connect([](std::string line) {
        std::cout << line << std::endl;
    });

    processManager.start ({"synergyc", "-f", "--debug", "DEBUG2", "192.168.3.30"});
    processManager.run();

    return 0;
};
