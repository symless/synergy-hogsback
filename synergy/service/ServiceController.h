#ifndef SERVICECONTROLLER_H
#define SERVICECONTROLLER_H

#include "synergy/service/ServiceWorker.h"
#include "synergy/service/IOService.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <thread>
#include <memory>
#include <string>

class ServiceControllerImp;

class ServiceController
{
public:
    ServiceController();
    ~ServiceController();

    void parseArg(int argc, char* argv[]) {
        for (int i = 1; i < argc; ++i) {
            std::string arg(argv[i]);
            if (arg == "/install" || arg == "--install") {
                m_install = true;
            }
            else if (arg == "/uninstall" || arg == "--uninstall") {
                m_uninstall = true;
            }
            else if (arg == "/foreground" || arg == "--foreground") {
                m_foreground = true;
            }
        }
    }

    virtual void run() {
        if (m_install) {
            install();
        }
        else if (m_uninstall) {
            uninstall();
        }
        else if (m_foreground) {
            runForeground();
        }
        else {
            doRun();
        }
    }

    void doRun();
    void install();
    void uninstall();
    void runForeground() {
        m_worker->start();
    }

private:
    void setupTerminationSignals() {
        m_terminationSignals.async_wait(
            boost::bind(&ServiceController::terminationSignalHandler, this, _1, _2));
    }

    void terminationSignalHandler(
            const boost::system::error_code& errorCode, int signalNumber) {
        if (errorCode == boost::asio::error::operation_aborted) {
            return;
        }

#if defined(SIGQUIT)
        if (signal_number == SIGINT || signal_number == SIGTERM || signal_number == SIGQUIT) {
#else
        if (signalNumber == SIGINT || signalNumber == SIGTERM) {
#endif
            shutdown();
        }
    }

    void shutdown();

protected:
    std::unique_ptr<ServiceControllerImp> m_imp;
    std::shared_ptr<ServiceWorker> m_worker;
    bool m_install;
    bool m_uninstall;
    bool m_foreground;
    boost::asio::io_service m_threadIoService;
    boost::asio::signal_set m_terminationSignals;
};

#endif // SERVICECONTROLLER_H
