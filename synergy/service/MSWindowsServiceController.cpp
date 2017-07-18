#include "MSWindowsServiceController.h"

#include <assert.h>
#include <sstream>

MSWindowsServiceController* MSWindowsServiceController::s_controller = NULL;

static char* kServiceDisplayName = "Synergy";

MSWindowsServiceController::MSWindowsServiceController() :
    m_statusHandle(NULL)
{

}

void MSWindowsServiceController::doRun()
{
    s_controller = this;

    SERVICE_TABLE_ENTRY serviceTable[] = {
        { processName(), MSWindowsServiceController::serviceMain },
        { NULL, NULL }
    };

    // Use main thread as the service control dispatcher
    StartServiceCtrlDispatcher(serviceTable);
}

void MSWindowsServiceController::manualInstall()
{
    // install default daemon if not already installed.
    if (!isDaemonInstalled(kServiceDisplayName)) {
        char path[MAX_PATH];
        GetModuleFileName(NULL, path, MAX_PATH);

        // wrap in quotes so a malicious user can't start \Program.exe as admin.
        std::stringstream pathStrStream;
        pathStrStream << '"';
        pathStrStream << path;
        pathStrStream << '"';

        // TODO: throw with error message
        SC_HANDLE manager = OpenSCManager(NULL, NULL, GENERIC_WRITE);
        if (manager == NULL) {
            // can't open service manager
            throw;
        }

        // create the service
        SC_HANDLE service = CreateService(
            manager,
            kServiceDisplayName,
            kServiceDisplayName,
            0,
            SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
            SERVICE_AUTO_START,
            SERVICE_ERROR_NORMAL,
            pathStrStream.str().c_str(),
            NULL,
            NULL,
            "",
            NULL,
            NULL);

        if (service == NULL) {
            // can't create service
            DWORD err = GetLastError();
            if (err != ERROR_SERVICE_EXISTS) {
                CloseServiceHandle(manager);
                throw;
            }
        }
        else {
            // done with service (but only try to close if not null)
            CloseServiceHandle(service);
        }

        // done with manager
        CloseServiceHandle(manager);
    }

    manualStart(kServiceDisplayName);
}

void MSWindowsServiceController::manualUninstall()
{

}

void MSWindowsServiceController::start(DWORD dwArgc, LPSTR *pszArgv)
{

}

void MSWindowsServiceController::stop()
{

}

void MSWindowsServiceController::pause()
{

}

void MSWindowsServiceController::resume()
{

}

void MSWindowsServiceController::shutdown()
{

}

bool MSWindowsServiceController::isDaemonInstalled(const char *name)
{
    // open service manager
    SC_HANDLE manager = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (manager == NULL) {
        return false;
    }

    // open the service
    SC_HANDLE service = OpenService(manager, name, GENERIC_READ);

    // clean up
    if (service != NULL) {
        CloseServiceHandle(service);
    }
    CloseServiceHandle(manager);

    return (service != NULL);
}

void MSWindowsServiceController::manualStart(const char *name)
{

}

void MSWindowsServiceController::serviceMain(DWORD dwArgc, LPSTR *pszArgv)
{
    assert(s_controller != NULL);

    // Register the handler function for the service
    s_controller->m_statusHandle = RegisterServiceCtrlHandler(
        s_controller->processName(), MSWindowsServiceController::serviceCtrlHandler);
    if (s_controller->m_statusHandle == NULL) {
        throw GetLastError();
    }

    // Start the service.
    s_controller->start(dwArgc, pszArgv);
}

void MSWindowsServiceController::serviceCtrlHandler(DWORD dwCtrl)
{
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP: s_controller->stop(); break;
    case SERVICE_CONTROL_PAUSE: s_controller->pause(); break;
    case SERVICE_CONTROL_CONTINUE: s_controller->resume(); break;
    case SERVICE_CONTROL_SHUTDOWN: s_controller->shutdown(); break;
    case SERVICE_CONTROL_INTERROGATE: break;
    default: break;
    }
}
