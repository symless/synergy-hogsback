#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "LibMacro.h"
#include "ScreenStatus.h"
#include "synergy/common/WampClient.h"

#include <boost/asio.hpp>
#include <QQuickItem>
#include <QProcess>
#include <thread>

class ScreenListModel;
class AppConfig;
class ConnectivityTester;
class RpcClient;

class LIB_SPEC ProcessManager : public QQuickItem
{
    Q_OBJECT

public:
    Q_PROPERTY(ConnectivityTester* connectivityTester READ connectivityTester WRITE setConnectivityTester)

    ProcessManager(RpcClient& rpcClient);
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
    void localInputDetected();

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

    boost::asio::io_service& m_ioService;
    RpcClient& m_rpcClient;
};

#endif // PROCESSMANAGER_H
