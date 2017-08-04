#ifndef SERVICECONTROLLER_H
#define SERVICECONTROLLER_H

#include <memory>
#include <string>
#include <windows.h>
#include <TlHelp32.h>

class ServiceController
{
public:
    ServiceController();
    ~ServiceController();

    void parseArg(int argc, char* argv[]);

    void run();

    void doRun();
    void install();
    void uninstall();
    void runForeground() {
        startSynergyd();
    }

private:
    void start(DWORD dwArgc, LPSTR *pszArgv);
    void stop();
    void pause();
    void resume();
    void shutdown();

    bool isDaemonInstalled(const char* name);
    void SetServiceStatus(DWORD currentState, DWORD win32ExitCode = NO_ERROR,
            DWORD waitHint = 0);

    void startSynergyd();
    void stopSynergyd();

    DWORD getActiveSession();
    HANDLE getElevateTokenInSession(DWORD sessionId, LPSECURITY_ATTRIBUTES security);
    void startSynergydAsUser(HANDLE userToken, LPSECURITY_ATTRIBUTES sa);
    void writeEventErrorLogEntry(char* message);
    bool findWinLogonInSession(PHANDLE process, DWORD sessionId);
    bool nextProcessEntry(HANDLE snapshot, LPPROCESSENTRY32 entry);
    HANDLE duplicateProcessToken(HANDLE process, LPSECURITY_ATTRIBUTES security);

    static void WINAPI serviceMain(DWORD dwArgc, LPSTR *pszArgv);
    static void WINAPI serviceCtrlHandler(DWORD dwCtrl);

protected:
    bool m_install;
    bool m_uninstall;
    bool m_foreground;

    SERVICE_STATUS m_status;
    SERVICE_STATUS_HANDLE m_statusHandle;

    static ServiceController* s_instance;
};

#endif // SERVICECONTROLLER_H
