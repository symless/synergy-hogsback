#include "ServiceController.h"

#include <Wtsapi32.h>
#include <Userenv.h>
#include <sstream>
#include <string>
#include <stdexcept>
#include <assert.h>

static char* kServiceProcessName = "synergy-controller";
static char* kServiceDisplayName = "Synergy";

ServiceController* ServiceController::s_instance = nullptr;

ServiceController::ServiceController() :
    m_install(false),
    m_uninstall(false),
    m_foreground(false)
{
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_START_PENDING;
    DWORD dwControlsAccepted = 0;
    dwControlsAccepted |= SERVICE_ACCEPT_STOP;
    dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
    dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
    m_status.dwControlsAccepted = dwControlsAccepted;
    m_status.dwWin32ExitCode = NO_ERROR;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;
}

ServiceController::~ServiceController()
{
}

void ServiceController::doRun()
{
    s_instance = this;

    SERVICE_TABLE_ENTRY serviceTable[] = {
        { kServiceProcessName, ServiceController::serviceMain },
        { NULL, NULL }
    };

    // Use main thread as the service control dispatcher
    StartServiceCtrlDispatcher(serviceTable);
}

void ServiceController::install()
{
    // install default daemon if not already installed.
    if (!isDaemonInstalled(kServiceProcessName)) {
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
            kServiceProcessName,
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
}

void ServiceController::uninstall()
{
    // open service manager
    SC_HANDLE manager = OpenSCManager(NULL, NULL, GENERIC_WRITE);
    if (manager == NULL) {
        throw std::runtime_error("can't open service manager for uninstalling");
    }

    SC_HANDLE service = OpenService(manager, kServiceProcessName, SERVICE_STOP |
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

void ServiceController::serviceMain(DWORD dwArgc, LPSTR *pszArgv)
{
    assert(s_instance != NULL);

    // Register the handler function for the service
    s_instance->m_statusHandle = RegisterServiceCtrlHandler(
        kServiceProcessName, ServiceController::serviceCtrlHandler);
    if (s_instance->m_statusHandle == NULL) {
        throw GetLastError();
    }

    // Start the service.
    s_instance->start(dwArgc, pszArgv);
}

void ServiceController::serviceCtrlHandler(DWORD dwCtrl)
{
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP: s_instance->stop(); break;
    case SERVICE_CONTROL_PAUSE: s_instance->pause(); break;
    case SERVICE_CONTROL_CONTINUE: s_instance->resume(); break;
    case SERVICE_CONTROL_SHUTDOWN: s_instance->shutdown(); break;
    case SERVICE_CONTROL_INTERROGATE: break;
    default: break;
    }
}


void ServiceController::start(DWORD dwArgc, LPSTR *pszArgv)
{
    try {
        SetServiceStatus(SERVICE_START_PENDING);

        startSynergyd();

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

void ServiceController::stop()
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

void ServiceController::pause()
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

void ServiceController::resume()
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

void ServiceController::shutdown()
{
    try {
       stopSynergyd();

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


bool ServiceController::isDaemonInstalled(const char *name)
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

void ServiceController::SetServiceStatus(DWORD currentState, DWORD win32ExitCode, DWORD waitHint)
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

void ServiceController::startSynergyd()
{
    DWORD sessionId = getActiveSession();
    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
    HANDLE token = getElevateTokenInSession(sessionId, &sa);
    startSynergydAsUser(token, &sa);
}

void ServiceController::stopSynergyd()
{

}

DWORD ServiceController::getActiveSession()
{
    return WTSGetActiveConsoleSessionId();
}

HANDLE ServiceController::getElevateTokenInSession(DWORD sessionId, LPSECURITY_ATTRIBUTES security)
{
    // TODO: get elevate token instead of normal token
    HANDLE sourceToken;
    WTSQueryUserToken(sessionId, &sourceToken);

    HANDLE newToken;
    DuplicateTokenEx(
        sourceToken, TOKEN_ASSIGN_PRIMARY | TOKEN_ALL_ACCESS, security,
        SecurityImpersonation, TokenPrimary, &newToken);

    return newToken;
}

void ServiceController::startSynergydAsUser(HANDLE userToken, LPSECURITY_ATTRIBUTES sa)
{
    std::string command("C:\\Projects\\build-synergy-v2-Desktop_Qt_5_8_0_MSVC2015_64bit-Debug\\bin\\synergyd.exe");
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = "winsta0\\Default";
    si.dwFlags |= STARTF_USESTDHANDLES;

    LPVOID environment;
    CreateEnvironmentBlock(&environment, userToken, FALSE);

    DWORD creationFlags =
        NORMAL_PRIORITY_CLASS |
        CREATE_NO_WINDOW |
        CREATE_UNICODE_ENVIRONMENT;

    // re-launch in current active user session
    BOOL createRet = CreateProcessAsUser(
        userToken, NULL, LPSTR(command.c_str()),
        sa, NULL, TRUE, creationFlags,
        environment, NULL, &si, &processInfo);

    DestroyEnvironmentBlock(environment);
    CloseHandle(userToken);
}
