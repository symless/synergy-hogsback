#ifndef CONNECTIVITYTESTER_H
#define CONNECTIVITYTESTER_H

#include "LibMacro.h"

#include <QQuickItem>
#include <QObject>

#include <QTimer>
#include <QSet>
#include <QQueue>

class CloudClient;
class QTcpServer;
class QTcpSocket;
class QThread;

class LIB_SPEC ConnectivityTester : public QObject
{
    Q_OBJECT
public:
    explicit ConnectivityTester(QObject* parent = 0);
    ~ConnectivityTester();

    Q_PROPERTY(CloudClient* cloudClient WRITE setCloudClient)

    void setCloudClient(CloudClient* cloudClient);

signals:
    void cloudClientSet();
    void startTesting();

private slots:
    void testNewScreens(QByteArray reply);
    void pollScreens();
    void onNewConnection();
    void onConnectionReadyRead();
    void onStartTesting();
    void onTestDelegateeDone(QList<bool> results);

private:
    QTimer m_timer;
    QSet<int> m_screenIdSet;
    QQueue<QString> m_pendingTestCases;
    QString m_localHostname;
    CloudClient* m_cloudClient;
    QTcpServer* m_tcpServer;
    QThread* m_testThread;
};

#endif // CONNECTIVITYTESTER_H
