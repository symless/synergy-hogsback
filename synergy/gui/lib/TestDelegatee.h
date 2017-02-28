#ifndef TESTDELEGATEE_H
#define TESTDELEGATEE_H

#include "LibMacro.h"

#include <QString>
#include <QObject>

class QTcpSocket;

extern const unsigned int kServerPort;

class LIB_SPEC TestDelegatee : public QObject
{
    Q_OBJECT
public:
    TestDelegatee(QString& testCase, QObject *parent = 0);

signals:
    void done(QList<bool>);

public slots:
    void start();
    void onConnected();
    void onReadyRead();
    void onSocketError();

private:
    QString m_testCase;
    QString m_screenId;
    QList<QString> m_ipList;
    QList<bool> m_results;
    QTcpSocket* m_tcpClient;
    int m_testIndex;
};

#endif // TESTDELEGATEE_H
