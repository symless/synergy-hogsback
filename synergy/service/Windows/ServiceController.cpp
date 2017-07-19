#include "../ServiceController.h"

#include <windows.h>
#include <sstream>
#include <string>
#include <stdexcept>
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
    void SetServiceStatus(DWORD currentState,
            DWORD win32ExitCode = NO_ERROR,
            DWORD waitHint = 0);

    void start(DWORD dwArgc, LPSTR *pszArgv);
    void stop();
    void pause();
    void resume();
    void shutdown();

    static void WINAPI serviceMain(DWORD dwArgc, LPSTR *pszArgv);
    static void WINAPI serviceCtrlHandler(DWORD dwCtrl);

private:
    SERVICE_STATUS m_status;
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

        SC_HANDLE manager = OpenSCManager(NULL, NULL, GENERIC_WRITE);
        if (manager == NULL) {
            throw std::runtime_error("can't open service manager for installing");
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

                std::ostringstream stream;
                stream << err;
                std::string errorMsg("can't create a service, error code: ");
                errorMsg += stream.str();
                throw std::runtime_error(errorMsg.c_str());
            }
        }
        else {
            CloseServiceHandle(service);
        }

        CloseServiceHandle(manager);
    }

    manualStart(kServiceDisplayName);
}

void ServiceControllerImp::manualUninstall()
{
    // open service manager
    SC_HANDLE manager = OpenSCManager(NULL, NULL, GENERIC_WRITE);
    if (manager == NULL) {
        throw std::runtime_error("can't open service manager for uninstalling");
    }

    SC_HANDLE service = OpenService(manager, kServiceDisplayName, SERVICE_STOP |
        SERVICE_QUERY_STATUS | DELETE);
    if (service == NULL) {
        DWORD err = GetLastError();
        CloseServiceHandle(manager);
        if (err != ERROR_SERVICE_DOES_NOT_EXIST) {
            std::ostringstream stream;
            stream << err;
            std::string errorMsg("can't uninstall a service, error code: ");
            errorMsg += stream.str();
            throw std::runtime_error(errorMsg.c_str());
        }
        throw std::runtime_error("trying to uninstall a non-existent service");
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
            CloseServiceHandle(service);
            CloseServiceHandle(manager);

            throw std::runtime_error("can't stop a service");
        }
    }

    if (!DeleteService(service)) {
        CloseServiceHandle(service);
        CloseServiceHandle(manager);

        throw std::runtime_error("can't delete a service");
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
    // open service manager
    SC_HANDLE manager = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (manager == NULL) {
        throw std::runtime_error("can't open service manager for starting a service");
    }

    // open the service
    SC_HANDLE service = OpenService(
        manager, name, SERVICE_START);

    if (service == NULL) {
        DWORD err = GetLastError();
        CloseServiceHandle(manager);

        std::ostringstream stream;
        stream << err;
        std::string errorMsg("can't open a service, error code: ");
        errorMsg += stream.str();
        throw std::runtime_error(errorMsg.c_str());
    }

    // start the service
    if (!StartService(service, 0, NULL)) {
        throw std::runtime_error("can't start a service");
    }
}

void ServiceControllerImp::SetServiceStatus(DWORD currentState, DWORD win32ExitCode, DWORD waitHint)
{
    static DWORD checkPoint = 1;

    m_status.dwCurrentState = currentState;
    m_status.dwWin32ExitCode = win32ExitCode;
    m_status.dwWaitHint = waitHint;

    m_status.dwCheckPoint =
        ((currentState == SERVICE_RUNNING) ||
        (currentState == SERVICE_STOPPED)) ?
        0 : checkPoint++;

    // Report the status of the service to the SCM.
    ::SetServiceStatus(m_statusHandle, &m_status);
}

void ServiceControllerImp::start(DWORD dwArgc, LPSTR *pszArgv)
{
    try {
        SetServiceStatus(SERVICE_START_PENDING);

        // TODO: Use another thread to do the actual work

        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (DWORD err) {
        SetServiceStatus(SERVICE_STOPPED, err);

        std::ostringstream stream;
        stream << err;
        std::string errorMsg("service failed to start, error code: ");
        errorMsg += stream.str();
        throw std::runtime_error(errorMsg.c_str());
    }
    catch (...) {
        SetServiceStatus(SERVICE_STOPPED);

        throw std::runtime_error("service failed to start");
    }
}

void ServiceControllerImp::stop()
{
    DWORD originalState = m_status.dwCurrentState;
    try {
        SetServiceStatus(SERVICE_STOP_PENDING);

        // TODO: Notify the worker to stop

        SetServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD err) {
        SetServiceStatus(originalState);

        std::ostringstream stream;
        stream << err;
        std::string errorMsg("service failed to stop, error code: ");
        errorMsg += stream.str();
        throw std::runtime_error(errorMsg.c_str());
    }
    catch (...) {
        SetServiceStatus(originalState);

        throw std::runtime_error("service failed to stop");
    }
}

void ServiceControllerImp::pause()
{
    try {
        SetServiceStatus(SERVICE_PAUSE_PENDING);

        // TODO: Notify the worker to pause

        SetServiceStatus(SERVICE_PAUSED);
    }
    catch (DWORD err) {
        SetServiceStatus(SERVICE_RUNNING, err);

        std::ostringstream stream;
        stream << err;
        std::string errorMsg("service failed to pause, error code: ");
        errorMsg += stream.str();
        throw std::runtime_error(errorMsg.c_str());
    }
    catch (...) {
        SetServiceStatus(SERVICE_RUNNING);

        throw std::runtime_error("service failed to pause");
    }
}

void ServiceControllerImp::resume()
{
    try {
        SetServiceStatus(SERVICE_CONTINUE_PENDING);

        // TODO: Notify the worker to resume

        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (DWORD err) {
        SetServiceStatus(SERVICE_PAUSED, err);

        std::ostringstream stream;
        stream << err;
        std::string errorMsg("service failed to resume, error code: ");
        errorMsg += stream.str();
        throw std::runtime_error(errorMsg.c_str());
    }
    catch (...) {
        SetServiceStatus(SERVICE_PAUSED);

        throw std::runtime_error("service failed to resume");
    }
}

void ServiceControllerImp::shutdown()
{
    try {
        // TODO: Notify the worker to shutdown

        SetServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD err) {
        std::ostringstream stream;
        stream << err;
        std::string errorMsg("service failed to shut down, error code: ");
        errorMsg += stream.str();
        throw std::runtime_error(errorMsg.c_str());
    }
    catch (...) {
        throw std::runtime_error("service failed to shut down");
    }
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
