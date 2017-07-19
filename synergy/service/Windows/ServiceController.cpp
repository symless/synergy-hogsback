#include "../ServiceController.h"

#include <windows.h>
#include <sstream>
#include <string>
#include <assert.h>

static char* kServiceProcessName = "synergy-service";
static char* kServiceDisplayName = "Synergy";

//
// ServiceControllerImp for Windows
//

class ServiceControllerImp {
public:
    ServiceControllerImp();
    ~ServiceControllerImp();

    void doRun();
    void manualInstall();
    void manualUninstall();

private:
    bool isDaemonInstalled(const char* name);
    void manualStart(const char* name);

    void start(DWORD dwArgc, LPSTR *pszArgv);
    void stop() {}
    void pause() {}
    void resume() {}
    void shutdown() {}

    static void WINAPI serviceMain(DWORD dwArgc, LPSTR *pszArgv);
    static void WINAPI serviceCtrlHandler(DWORD dwCtrl);

private:
    SERVICE_STATUS_HANDLE m_statusHandle;

    static ServiceControllerImp* s_impInstance;
};

ServiceControllerImp* ServiceControllerImp::s_impInstance = NULL;

ServiceControllerImp::ServiceControllerImp()
{

}

ServiceControllerImp::~ServiceControllerImp()
{

}

void ServiceControllerImp::doRun()
{
    s_impInstance = this;

    SERVICE_TABLE_ENTRY serviceTable[] = {
        { kServiceProcessName, ServiceControllerImp::serviceMain },
        { NULL, NULL }
    };

    // Use main thread as the service control dispatcher
    StartServiceCtrlDispatcher(serviceTable);
}

void ServiceControllerImp::manualInstall()
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

void ServiceControllerImp::manualUninstall()
{
    // TODO: throw with error message
    // open service manager
    SC_HANDLE manager = OpenSCManager(NULL, NULL, GENERIC_WRITE);
    if (manager == NULL) {
        // can't open service manager
        throw;
    }

    SC_HANDLE service = OpenService(manager, kServiceDisplayName, SERVICE_STOP |
        SERVICE_QUERY_STATUS | DELETE);
    if (service == NULL) {
        DWORD err = GetLastError();
        CloseServiceHandle(manager);
        if (err != ERROR_SERVICE_DOES_NOT_EXIST) {
            throw;
        }
        throw;
    }

    // Try to stop the service
    SERVICE_STATUS ssSvcStatus = {};
    if (ControlService(service, SERVICE_CONTROL_STOP, &ssSvcStatus)) {
        Sleep(1000);

        while (QueryServiceStatus(service, &ssSvcStatus)) {
            if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                Sleep(1000);
            }
            else break;
        }

        if (ssSvcStatus.dwCurrentState != SERVICE_STOPPED) {
            throw;
        }
    }

    if (!DeleteService(service)) {
        throw;
    }

    // clean up
    CloseServiceHandle(service);
    CloseServiceHandle(manager);
}

bool ServiceControllerImp::isDaemonInstalled(const char *name)
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

void ServiceControllerImp::manualStart(const char *name)
{
    // TODO: throw with error message
    // open service manager
    SC_HANDLE manager = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (manager == NULL) {
        throw;
    }

    // open the service
    SC_HANDLE service = OpenService(
        manager, name, SERVICE_START);

    if (service == NULL) {
        CloseServiceHandle(manager);
        throw;
    }

    // start the service
    if (!StartService(service, 0, NULL)) {
        throw;
    }
}

void ServiceControllerImp::start(DWORD dwArgc, LPSTR *pszArgv)
{
    // TODO: Use another thread to do the actual work
}

void ServiceControllerImp::serviceMain(DWORD dwArgc, LPSTR *pszArgv)
{
    assert(s_impInstance != NULL);

    // Register the handler function for the service
    s_impInstance->m_statusHandle = RegisterServiceCtrlHandler(
        kServiceProcessName, ServiceControllerImp::serviceCtrlHandler);
    if (s_impInstance->m_statusHandle == NULL) {
        throw GetLastError();
    }

    // Start the service.
    s_impInstance->start(dwArgc, pszArgv);
}

void ServiceControllerImp::serviceCtrlHandler(DWORD dwCtrl)
{
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP: s_impInstance->stop(); break;
    case SERVICE_CONTROL_PAUSE: s_impInstance->pause(); break;
    case SERVICE_CONTROL_CONTINUE: s_impInstance->resume(); break;
    case SERVICE_CONTROL_SHUTDOWN: s_impInstance->shutdown(); break;
    case SERVICE_CONTROL_INTERROGATE: break;
    default: break;
    }
}

//
// ServiceController for Windows
//

ServiceController::ServiceController() :
    m_install(false),
    m_uninstall(false)
{
    m_imp = std::make_unique<ServiceControllerImp>();
}

ServiceController::~ServiceController()
{

}

void ServiceController::doRun()
{
    m_imp->doRun();
}

void ServiceController::install()
{
    m_imp->manualInstall();
}

void ServiceController::uninstall()
{
    m_imp->manualUninstall();
}
