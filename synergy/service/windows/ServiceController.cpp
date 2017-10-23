#include "ServiceController.h"

#include "synergy/common/DirectoryManager.h"

#include <Wtsapi32.h>
#include <Userenv.h>
#include <sstream>
#include <string>
#include <list>
#include <stdexcept>
#include <assert.h>

static char* kServiceControllerName = "synergy-service-controller";
static char* kServiceControllerDisplayName = "Synergy";
static char* kServiceProcess = "synergy-service.exe";
static char* kCoreProcess = "synergy-core.exe";
static char* kWinLogon = "winlogon.exe";
static const UINT kExitCode = 0;
static const DWORD kMonitorIntervalMS = 2000;
ServiceController* ServiceController::s_instance = nullptr;

ServiceController::ServiceController() :
    m_install(false),
    m_uninstall(false),
    m_foreground(false),
    m_jobOject(NULL),
    m_monitorThread(NULL)
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

void ServiceController::parseArg(int argc, char *argv[])
{
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

void ServiceController::run()
{
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

void ServiceController::doRun()
{
    s_instance = this;

    SERVICE_TABLE_ENTRY serviceTable[] = {
        { kServiceControllerName, ServiceController::serviceMain },
        { NULL, NULL }
    };

    // Use main thread as the service control dispatcher
    StartServiceCtrlDispatcher(serviceTable);
}

void ServiceController::install()
{
    // install default daemon if not already installed.
    if (!isDaemonInstalled(kServiceControllerName)) {
        char path[MAX_PATH];
        GetModuleFileName(NULL, path, MAX_PATH);

        // wrap in quotes so a malicious user can't start \Program.exe as admin.
        std::stringstream pathStrStream;
        pathStrStream << '"';
        pathStrStream << path;
        pathStrStream << '"';

        SC_HANDLE manager = OpenSCManager(NULL, NULL, GENERIC_WRITE);
        if (manager == NULL) {
            writeEventErrorLog("Failed to open service manager for installing");
            throw;
        }

        // create the service
        SC_HANDLE service = CreateService(
            manager,
            kServiceControllerName,
            kServiceControllerDisplayName,
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
            DWORD err = GetLastError();
            if (err != ERROR_SERVICE_EXISTS) {
                CloseServiceHandle(manager);

                std::ostringstream stream;
                stream << err;
                std::string errorMsg("Failed to create the service, error code: ");
                errorMsg += stream.str();

                writeEventErrorLog(errorMsg.c_str());
                throw;
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
        writeEventErrorLog("Failed to open service manager for uninstalling");
        throw;
    }

    SC_HANDLE service = OpenService(manager, kServiceControllerName, SERVICE_STOP |
        SERVICE_QUERY_STATUS | DELETE);
    if (service == NULL) {
        DWORD err = GetLastError();
        CloseServiceHandle(manager);

        if (err != ERROR_SERVICE_DOES_NOT_EXIST) {
            std::ostringstream stream;
            stream << err;
            std::string errorMsg("Failed to uninstall the service, error code: ");
            errorMsg += stream.str();

            writeEventErrorLog(errorMsg.c_str());
            throw;
        }
        writeEventErrorLog("Service doesn't exist");
        throw;
    }

    // Try to stop the service
    SERVICE_STATUS serviceStatus = {};
    if (ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus)) {
        Sleep(1000);

        while (QueryServiceStatus(service, &serviceStatus)) {
            if (serviceStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                Sleep(1000);
            }
            else break;
        }

        if (serviceStatus.dwCurrentState != SERVICE_STOPPED) {
            CloseServiceHandle(service);
            CloseServiceHandle(manager);

            writeEventErrorLog("Failed to stop the service");
            throw;
        }
    }

    if (!DeleteService(service)) {
        CloseServiceHandle(service);
        CloseServiceHandle(manager);

        writeEventErrorLog("Failed to delete the service");
        throw;
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
        kServiceControllerName, ServiceController::serviceCtrlHandler);
    if (s_instance->m_statusHandle == NULL) {
        s_instance->writeEventErrorLog("Failed to register service control handler");
        throw;
    }

    // Start the service.
    s_instance->start(dwArgc, pszArgv);
}

void ServiceController::serviceCtrlHandler(DWORD dwCtrl)
{
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        s_instance->stop();
        break;
    case SERVICE_CONTROL_PAUSE:
        s_instance->pause();
        break;
    case SERVICE_CONTROL_CONTINUE:
        s_instance->resume();
        break;
    case SERVICE_CONTROL_SHUTDOWN:
        s_instance->shutdown();
        break;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    default:
        break;
    }
}

void
ServiceController::monitorService()
{
    writeEventErrorLog("monitoring");
    while(true) {
        Sleep(kMonitorIntervalMS);
        writeEventErrorLog("checking");
        if (m_serviceHandle) {
            DWORD exitCode;
            GetExitCodeProcess(m_serviceHandle, &exitCode);

            if (exitCode != STILL_ACTIVE) {
                writeEventErrorLog("Detect Synergy service is not running, restarting");
                // HACK: use exception to restart the controller
                throw;
            }
            else {
                writeEventErrorLog("service is running");
            }
        }
        else {
            if (!m_serviceHandle)
                writeEventErrorLog("serviceHandle not ready");
            if (!m_jobOject)
                writeEventErrorLog("jobOject not ready");
        }
    }
}

DWORD
ServiceController::staticMonitorService(LPVOID)
{
    if (!s_instance) {
        return -1;
    }

    s_instance->monitorService();

    return 0;
}

void ServiceController::start(DWORD dwArgc, LPSTR *pszArgv)
{
    try {
        setServiceStatus(SERVICE_START_PENDING);

        startSynergyService();

        setServiceStatus(SERVICE_RUNNING);
    }
    catch (DWORD err) {
        setServiceStatus(SERVICE_STOPPED, err);

        std::ostringstream stream;
        stream << err;
        std::string errorMsg("Failed to start the service, error code: ");
        errorMsg += stream.str();

        writeEventErrorLog(errorMsg.c_str());
    }
    catch (...) {
        setServiceStatus(SERVICE_STOPPED);

        writeEventErrorLog("Failed to start the service");
    }
}

void ServiceController::stop()
{
    DWORD originalState = m_status.dwCurrentState;
    try {
        setServiceStatus(SERVICE_STOP_PENDING);

        stopSynergyService();

        setServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD err) {
        setServiceStatus(originalState);

        std::ostringstream stream;
        stream << err;
        std::string errorMsg("Failed to stop the service, error code: ");
        errorMsg += stream.str();

        writeEventErrorLog(errorMsg.c_str());
    }
    catch (...) {
        setServiceStatus(originalState);

        writeEventErrorLog("Failed to stop the service");
    }
}

void ServiceController::pause()
{
    try {
        setServiceStatus(SERVICE_PAUSE_PENDING);

        // TODO: Notify the worker to pause

        setServiceStatus(SERVICE_PAUSED);
    }
    catch (DWORD err) {
        setServiceStatus(SERVICE_RUNNING, err);

        std::ostringstream stream;
        stream << err;
        std::string errorMsg("Failed to pause the service, error code: ");
        errorMsg += stream.str();

        writeEventErrorLog(errorMsg.c_str());
    }
    catch (...) {
        setServiceStatus(SERVICE_RUNNING);

        writeEventErrorLog("Failed to pause the service");
    }
}

void ServiceController::resume()
{
    try {
        setServiceStatus(SERVICE_CONTINUE_PENDING);

        // TODO: Notify the worker to resume

        setServiceStatus(SERVICE_RUNNING);
    }
    catch (DWORD err) {
        setServiceStatus(SERVICE_PAUSED, err);

        std::ostringstream stream;
        stream << err;
        std::string errorMsg("Failed to resume the service, error code: ");
        errorMsg += stream.str();

        writeEventErrorLog(errorMsg.c_str());
    }
    catch (...) {
        setServiceStatus(SERVICE_PAUSED);

        writeEventErrorLog("Failed to resume the service");
    }
}

void ServiceController::shutdown()
{
    try {
        stopSynergyService();

        setServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD err) {
        std::ostringstream stream;
        stream << err;
        std::string errorMsg("Failed to shut down the service, error code: ");
        errorMsg += stream.str();

        writeEventErrorLog(errorMsg.c_str());
    }
    catch (...) {
        writeEventErrorLog("Failed to shut down the service");
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

void ServiceController::setServiceStatus(DWORD currentState, DWORD win32ExitCode, DWORD waitHint)
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

void ServiceController::startSynergyService()
{
    writeEventErrorLog("startSynergyService");
    try {
        DWORD sessionId = getActiveSession();
        SECURITY_ATTRIBUTES securityAttributes;
        ZeroMemory(&securityAttributes, sizeof(SECURITY_ATTRIBUTES));
        HANDLE token = getElevateTokenInSession(sessionId, &securityAttributes);
        startSynergyServiceAsUser(token, &securityAttributes);

        // Use separate thread to monitor underlying service process
        if (!m_monitorThread) {
            writeEventErrorLog("starting thread");
            DWORD threadId;
            m_monitorThread = CreateThread(NULL, 0, &ServiceController::staticMonitorService, this, 0, &threadId);
        }
    }
    catch (...) {
        writeEventErrorLog("Failed to start synergy service");
    }
}

void ServiceController::stopSynergyService()
{
    if (m_monitorThread) {
        writeEventErrorLog("terminate thread");
        TerminateThread(m_monitorThread, 0);
        CloseHandle(m_monitorThread);
        m_monitorThread = nullptr;
    }

    if (m_jobOject) {
        BOOL result = TerminateJobObject(m_jobOject, kExitCode);
        if (!result) {
            writeEventErrorLog("Failed to terminate synergy service group");
        }
        else {
            stopAllUserProcesses();
        }

        m_jobOject = nullptr;
    }

    return;
}

DWORD ServiceController::getActiveSession()
{
    return WTSGetActiveConsoleSessionId();
}

HANDLE ServiceController::getElevateTokenInSession(DWORD sessionId, LPSECURITY_ATTRIBUTES security)
{
    HANDLE process;
    if (!findProcessInSession(kWinLogon, &process, sessionId)) {
        std::ostringstream stream;
        stream << sessionId;
        std::string errorMsg("Failed to find ");
        errorMsg += kWinLogon;
        errorMsg += " in session ";
        errorMsg += stream.str();

        writeEventErrorLog(errorMsg.c_str());
        throw;
    }

    return duplicateProcessToken(process, security);
}

void ServiceController::startSynergyServiceAsUser(HANDLE userToken, LPSECURITY_ATTRIBUTES sa)
{
    auto const command = (DirectoryManager::instance()->installDir()  / kServiceProcess).string();

    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = "winsta0\\Default";
    si.dwFlags |= STARTF_USESTDHANDLES;

    m_jobOject = CreateJobObject(NULL, "SynergyJob");

    LPVOID environment;
    BOOL createRet = CreateEnvironmentBlock(&environment, userToken, FALSE);
    if (!createRet) {
        writeEventErrorLog("Failed to create environment block");
        throw;
    }

    DWORD creationFlags =
        NORMAL_PRIORITY_CLASS |
        CREATE_NO_WINDOW |
        CREATE_UNICODE_ENVIRONMENT |
        CREATE_NEW_PROCESS_GROUP;

    // re-launch in current active user session
    createRet = CreateProcessAsUser(
        userToken, NULL, LPSTR(command.c_str()),
        sa, NULL, TRUE, creationFlags,
        environment, NULL, &si, &processInfo);

    if (!createRet) {
        writeEventErrorLog("Failed to create the service in user session");
        throw;
    }

    createRet = AssignProcessToJobObject(m_jobOject, processInfo.hProcess);
    if (!createRet) {
        writeEventErrorLog("Failed to assign the service in a group");
        throw;
    }

    m_serviceHandle = processInfo.hProcess;

    DestroyEnvironmentBlock(environment);
    CloseHandle(userToken);
}

void ServiceController::writeEventErrorLog(const char* message)
{
    WORD type = EVENTLOG_ERROR_TYPE;

    HANDLE eventSource = NULL;
    LPCSTR strings[2] = {NULL, NULL};

    eventSource = RegisterEventSource(NULL, kServiceControllerName);
    if (eventSource) {
        strings[0] = kServiceControllerName;
        strings[1] = message;

        ReportEvent(eventSource,  // Event log handle
            type,                 // Event type
            0,                     // Event category
            0,                     // Event identifier
            NULL,                  // No security identifier
            2,                     // Size of lpszStrings array
            0,                     // No binary data
            strings,           // Array of strings
            NULL                   // No binary data
            );

        DeregisterEventSource(eventSource);
    }
}

bool
ServiceController::findProcessInSession(const char* processName, PHANDLE process, DWORD sessionId)
{
    // take a snapshot of the running processes
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    // get the first process, and if we can't do that then it's
    // unlikely we can go any further
    BOOL gotEntry = Process32First(snapshot, &entry);
    if (!gotEntry) {
        return false;
    }

    DWORD pid = 0;
    while (gotEntry) {
        // make sure we're not checking the system process
        if (entry.th32ProcessID != 0) {
            DWORD processSessionId;
            BOOL pidToSidRet = ProcessIdToSessionId(
                entry.th32ProcessID, &processSessionId);

            if (!pidToSidRet) {
                // if we can not acquire session associated with a specified process,
                // simply ignore it

                gotEntry = nextProcessEntry(snapshot, &entry);
                continue;
            }
            else {
                if (processSessionId == sessionId) {
                    if (_stricmp(entry.szExeFile, processName) == 0) {
                        // found the target process
                        pid = entry.th32ProcessID;
                        break;
                    }
                }
            }

        }

        // now move on to the next entry (if we're not at the end)
        gotEntry = nextProcessEntry(snapshot, &entry);
    }

    CloseHandle(snapshot);

    if (pid) {
        if (process != NULL) {
            // now get the process
            *process = OpenProcess(MAXIMUM_ALLOWED, FALSE, pid);
        }
        return true;
    }
    else {
        return false;
    }
}

bool
ServiceController::nextProcessEntry(HANDLE snapshot, LPPROCESSENTRY32 entry)
{
    return Process32Next(snapshot, entry) ? true : false;
}

HANDLE
ServiceController::duplicateProcessToken(HANDLE process, LPSECURITY_ATTRIBUTES security)
{
    HANDLE sourceToken;

    BOOL tokenRet = OpenProcessToken(
        process,
        TOKEN_ASSIGN_PRIMARY | TOKEN_ALL_ACCESS,
        &sourceToken);

    if (!tokenRet) {
        writeEventErrorLog("Failed to open process token");
        throw;
    }

    HANDLE newToken;
    BOOL duplicateRet = DuplicateTokenEx(
        sourceToken, TOKEN_ASSIGN_PRIMARY | TOKEN_ALL_ACCESS, security,
        SecurityImpersonation, TokenPrimary, &newToken);

    if (!duplicateRet) {
        writeEventErrorLog("Failed to duplicate process token");
        throw;
    }

    return newToken;
}

void ServiceController::stopAllUserProcesses()
{
    std::vector<std::string> processes;
    processes.emplace_back(kServiceProcess);
    processes.emplace_back(kCoreProcess);

    for (auto process : processes) {
        HANDLE processHandle = NULL;
        if (findProcessInSession(process.c_str(), &processHandle, getActiveSession())) {
            if (!TerminateProcess(processHandle, kExitCode)) {
                std::string errorMsg("Failed to shutdown ");
                errorMsg += process;
                writeEventErrorLog(errorMsg.c_str());
            }
        }
    }
}
