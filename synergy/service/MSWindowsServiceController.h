#ifndef MSWINDOWSSERVICECONTROLLER_H
#define MSWINDOWSSERVICECONTROLLER_H

#include "ServiceController.h"

#include <windows.h>

class MSWindowsServiceController : public ServiceController
{
public:
    MSWindowsServiceController();

    // IServiceImp
    virtual void doRun();
    virtual void install() { manualInstall(); };
    virtual void uninstall() { manualUninstall(); };

private:
    static void WINAPI serviceMain(DWORD dwArgc, LPSTR *pszArgv);
    static void WINAPI serviceCtrlHandler(DWORD dwCtrl);
    void start(DWORD dwArgc, LPSTR *pszArgv);
    void stop();
    void pause();
    void resume();
    void shutdown();
    bool isDaemonInstalled(const char* name);
    void manualStart(const char* name);
    void manualInstall();
    void manualUninstall();

private:
    SERVICE_STATUS_HANDLE m_statusHandle;

    static MSWindowsServiceController* s_controller;
};

#endif // MSWINDOWSSERVICECONTROLLER_H
