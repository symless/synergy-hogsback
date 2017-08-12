#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "LibMacro.h"
#include "synergy/common/ScreenStatus.h"

#include <boost/asio.hpp>
#include <QQuickItem>
#include <QProcess>
#include <thread>
#include <memory>
#include <utility>

class ScreenListModel;
class AppConfig;
class ConnectivityTester;
class WampClient;

class LIB_SPEC ProcessManager : public QQuickItem
{
    Q_OBJECT

public:
    Q_PROPERTY(ConnectivityTester* connectivityTester READ connectivityTester WRITE setConnectivityTester)

    ProcessManager();
    ProcessManager(WampClient&);
    ~ProcessManager();
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
    void screenStatusChanged(QPair<QString, ScreenStatus>);
    void rpcScreenStatusChanged(QString, int);
    void localInputDetected();
    void logCoreOutput(QString);

public slots:
    void newServerDetected(int serverId);
    void onRpcScreenStatusChanged(QString, int);

private slots:
    void exit(int exitCode, QProcess::ExitStatus);
    void onLogCoreOutput(QString text);
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
    std::shared_ptr<WampClient> m_wampClient;
};

#endif // PROCESSMANAGER_H
