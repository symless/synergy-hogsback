#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "LibMacro.h"
#include "ScreenState.h"

#include <QQuickItem>
#include <QProcess>

class ScreenListModel;
class AppConfig;
class ConnectivityTester;

class LIB_SPEC ProcessManager : public QQuickItem
{
    Q_OBJECT

public:
    Q_PROPERTY(ConnectivityTester* connectivityTester READ connectivityTester WRITE setConnectivityTester)

    ProcessManager();
    Q_INVOKABLE void start();
    int processMode();
    void setProcessMode(int mode);
    bool active();
    void setActive(bool active);
    ConnectivityTester* connectivityTester();
    void setConnectivityTester(ConnectivityTester* tester);

    QString serverIp() const;
    void setServerIp(const QString& serverIp);

signals:
    void screenStateChanged(QPair<QString, ScreenState>);

public slots:
     void newServerDetected(int serverId);

private slots:
    void exit(int exitCode, QProcess::ExitStatus);
    void logCoreOutput();
    void logCoreError();

private:
    void startProcessOnWin();
    void startProcess();

private:
    QProcess* m_process;
    AppConfig* m_appConfig;
    ConnectivityTester* m_connectivityTester;
    int m_processMode;
    bool m_active;
    QString m_serverIp;
};

#endif // PROCESSMANAGER_H
