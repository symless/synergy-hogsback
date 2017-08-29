#include <synergy/common/CrashHandler.h>
#include <synergy/service/ServiceWorker.h>
#include <synergy/service/TerminationSignalListener.h>

#include <boost/asio/io_service.hpp>
#include <cstdlib>
#include <exception>
#include <iostream>

static bool crashHandlerIsInstalled = false;

int
main (int argc, char* argv[]) {
    crashHandlerIsInstalled = startCrashHandler();

    boost::asio::io_service ioService;

    ServiceWorker serviceWorker(ioService);

    TerminationSignalListener signalListener(ioService);
    signalListener.setHandler([&serviceWorker] () {
        serviceWorker.shutdown();
    });

    serviceWorker.start();
    return EXIT_SUCCESS;

}
