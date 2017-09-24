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
main (int argc, char* argv[]) {

    // cache the directory of this binary for installedDir
    boost::filesystem::path selfPath = boost::filesystem::system_complete(argv[0]);
    DirectoryManager::instance()->m_programDir = selfPath.remove_filename().string();
    // TODO: figure out linux linker error and move boost path stuff back to init
    //DirectoryManager::instance()->init(argc, argv);

    try {
        startCrashHandler();
    }
    catch (const std::exception& ex) {
        serviceLog()->error("failed to start crash handler: {}", ex.what());
    }
    catch (...) {
        serviceLog()->error("failed to start crash handler: unknown error");
    }

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
