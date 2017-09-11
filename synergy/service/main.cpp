#include <synergy/common/CrashHandler.h>
#include <synergy/common/UserConfig.h>
#include <synergy/common/DirectoryManager.h>
#include <synergy/service/ServiceWorker.h>
#include <synergy/service/TerminationSignalListener.h>
#include <synergy/service/Logs.h>
#include <boost/asio/io_service.hpp>
#include <cstdlib>
#include <exception>
#include <iostream>

int
main (int argc, char* argv[]) {

    startCrashHandler();

    mainLog()->info("starting service...");

    // cache the directory of this binary for installedDir
    auto selfPath = boost::filesystem::system_complete(argv[0]);
    g_programDir = selfPath.remove_filename().string();

    boost::asio::io_service ioService;

    auto userConfig = std::make_shared<UserConfig>();
    userConfig->load();
    ServiceWorker serviceWorker(ioService, userConfig);

    TerminationSignalListener signalListener(ioService);
    signalListener.setHandler([&serviceWorker] () {
        serviceWorker.shutdown();
    });

    serviceWorker.start();
    return EXIT_SUCCESS;
}
