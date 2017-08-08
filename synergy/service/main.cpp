#include <synergy/common/CrashHandler.h>
#include "ServiceWorker.h"
#include "TerminationSignalListener.h"

#include <boost/asio/io_service.hpp>

int
main (int argc, char* argv[]) {
    startCrashHandler();

    boost::asio::io_service ioService;
    ServiceWorker serviceWorker(ioService);
    TerminationSignalListener signalListener(ioService);
    signalListener.setHandler([&serviceWorker] () {
        serviceWorker.shutdown();
    });

    try {
        serviceWorker.start();
    }
    catch (...) {
        return 1;
    }

    return 0;
};
