#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "LibMacro.h"

#include <QQuickItem>
#include <QProcess>

class ScreenListModel;

class LIB_SPEC ProcessManager : public QQuickItem
{
    Q_OBJECT

public:
    ProcessManager();
    Q_INVOKABLE void start();
    int processMode();
    void setProcessMode(int mode);
    bool active();
    void setActive(bool active);

    QString serverIp() const;
    void setServerIp(const QString& serverIp);

private slots:
    void exit(int exitCode, QProcess::ExitStatus);
    void logCoreOutput();
    void logCoreError();

private:
    void startProcessOnWin();
    void startProcess();

private:
    QProcess* m_process;
    int m_processMode;
    bool m_active;
    QString m_serverIp;
};

#endif // PROCESSMANAGER_H
