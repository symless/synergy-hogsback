#include <synergy/common/CrashHandler.h>
#include "ServiceController.h"

int
main (int argc, char* argv[]) {
    startCrashHandler();

    ServiceController serviceController;
    serviceController.parseArg(argc, argv);
    try {
        serviceController.run();
    }
    catch (...) {
        return 1;
    }

    return 0;
};
