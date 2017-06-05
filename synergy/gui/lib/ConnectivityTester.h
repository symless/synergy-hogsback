#ifndef CONNECTIVITYTESTER_H
#define CONNECTIVITYTESTER_H

#include "LibMacro.h"

#include <QQuickItem>
#include <QObject>

#include <QTimer>
#include <QSet>
#include <QList>

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

    QStringList getSuccessfulResults(int screenId) const;
signals:
    void startTesting();

private slots:
    void testNewScreens(QByteArray reply);
    void pollScreens();
    void onNewConnection();
    void onConnectionReadyRead();
    void onStartTesting();
    void onTestDelegateeDone(QMap<QString, bool> results);

private:
    QTimer m_timer;
    QSet<int> m_screenIdSet;
    QList<QString> m_pendingTestCases;
    QString m_localHostname;
    CloudClient* m_cloudClient;
    QTcpServer* m_tcpServer;
    QThread* m_testThread;
    int m_testCaseBatchSize;
    QMap<int, QStringList> m_screenSuccessfulResults;
};

#endif // CONNECTIVITYTESTER_H
