#include <synergy/service/App.h>

#include <synergy/common/CrashHandler.h>
#include <synergy/common/UserConfig.h>
#include <synergy/common/DirectoryManager.h>
#include <synergy/service/ServiceWorker.h>
#include <synergy/service/TerminationSignalListener.h>
#include <synergy/service/ServiceLogs.h>
#include <boost/asio/io_service.hpp>
#include <cstdlib>
#include <exception>
#include <iostream>

int
App::run(int argc, char* argv[])
{
    try {
        DirectoryManager::instance()->init(argv[0]);
        auto installDir = DirectoryManager::instance()->installDir().string();
        serviceLog()->debug("install dir: {}", installDir);
    }
    catch (const std::exception& ex) {
        serviceLog()->error("failed to init dirs: {}", ex.what());
        return EXIT_FAILURE;
    }
    catch (...) {
        serviceLog()->error("failed to init dirs: unknown error");
        return EXIT_FAILURE;
    }

    try {
        startCrashHandler();
    }
    catch (const std::exception& ex) {
        serviceLog()->error("failed to start crash handler: {}", ex.what());
    }
    catch (...) {
        serviceLog()->error("failed to start crash handler: unknown error");
    }


    try {
        serviceLog()->info("starting service...");

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
    catch (const std::exception& ex) {
        serviceLog()->error("failed to start service: {}", ex.what());
        return EXIT_FAILURE;
    }
    catch (...) {
        serviceLog()->error("failed to start service: unknown error");
        return EXIT_FAILURE;
    }
}
