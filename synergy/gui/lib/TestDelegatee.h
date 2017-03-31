#ifndef TESTDELEGATEE_H
#define TESTDELEGATEE_H

#include "LibMacro.h"

#include <QString>
#include <QList>
#include <QMap>
#include <QObject>

class QTcpSocket;

extern const unsigned int kServerPort;

class LIB_SPEC TestDelegatee : public QObject
{
    Q_OBJECT
public:
    TestDelegatee(const QList<QString>& testCases, int batchSize, QObject* parent = 0);
    ~TestDelegatee();

signals:
    void done(QMap<QString, bool>);

public slots:
    void start();
    void onConnected();
    void onReadyRead();
    void onTestFinish();

private:
    void cleanUp();

private:
    QList<QString> m_ipList;
    QMap<QTcpSocket*, QString> m_socketIpMap;
    QMap<QString, bool> m_results;
};

#endif // TESTDELEGATEE_H
