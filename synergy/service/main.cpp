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

void
loadInstallDir(const char* argv0)
{
#ifdef __linux__
    char buffer[1024] = {0};
    ssize_t size = readlink("/proc/self/exe", buffer, sizeof(buffer));
    if (size == 0 || size == sizeof(buffer)) {
        return;
    }
    std::string path(buffer, size);
    boost::system::error_code ec;
    auto selfPath = boost::filesystem::canonical(
        path, boost::filesystem::current_path(), ec);
#else
    auto selfPath = boost::filesystem::system_complete(argv0);
#endif

    DirectoryManager::instance()->m_programDir = selfPath.remove_filename().string();
    serviceLog()->debug("install dir: {}", DirectoryManager::instance()->m_programDir);
}

int
main (int argc, char* argv[]) {

    // cache the directory of this binary for installedDir
    loadInstallDir(argv[0]);

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

    try {
        ServiceWorker serviceWorker(ioService, userConfig);

        TerminationSignalListener signalListener(ioService);
        signalListener.setHandler([&serviceWorker] () {
            serviceWorker.shutdown();
        });

        serviceWorker.start();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& ex) {
        serviceLog()->error("service worker failed: {}", ex.what());
        return EXIT_FAILURE;
    }
}
