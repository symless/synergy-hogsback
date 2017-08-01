#include <synergy/common/CrashHandler.h>
#include "ServiceController.h"

int
main (int argc, char* argv[]) {
    startCrashHandler();

    char* volatile x = 0;
    *x = 'a';

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
