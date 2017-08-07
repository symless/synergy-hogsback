#include <synergy/common/CrashHandler.h>
#include "ServiceWorker.h"

#include <boost/asio/io_service.hpp>

int
main (int argc, char* argv[]) {
    startCrashHandler();

    boost::asio::io_service ioService;
    ServiceWorker serviceWorker(ioService);

    try {
        serviceWorker.start();
    }
    catch (...) {
        return 1;
    }

    return 0;
};
