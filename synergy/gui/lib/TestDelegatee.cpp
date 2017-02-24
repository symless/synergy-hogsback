#include "TestDelegatee.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QDataStream>

const unsigned int kServerPort = 24810;

TestDelegatee::TestDelegatee(QString& testCase, QObject *parent) :
    QObject(parent),
    m_testCase(testCase),
    m_tcpclient(new QTcpSocket()),
    m_testIndex(0)

{
    QStringList parts;
    parts = m_testCase.split(',');

    if (parts.size() > 1) {
        m_screenId = parts[0];
        for (int i = 1; i < parts.size(); i++) {
            m_ipList.append(parts[i]);
            m_results.append(false);
        }
    }

    connect(m_tcpclient, &QAbstractSocket::connected, this, &TestDelegatee::onConnected);
    connect(m_tcpclient, &QIODevice::readyRead, this, &TestDelegatee::onReadyRead);
    typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
    connect(m_tcpclient, static_cast<QAbstractSocketErrorSignal>(&QAbstractSocket::error),
        this, &TestDelegatee::onSocketError);
}

void TestDelegatee::start()
{
    if (m_testIndex >= m_ipList.size()) {
        emit done(m_results);
    }

    QHostAddress address(m_ipList[m_testIndex]);
    m_tcpclient->connectToHost(address, kServerPort);

    return;
}

void TestDelegatee::onConnected()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_7);
    out << "ClientHello";
    m_tcpclient->write(block);
}

void TestDelegatee::onReadyRead()
{
    QDataStream in;
    in.setDevice(m_tcpclient);
    in.setVersion(QDataStream::Qt_5_7);
    in.startTransaction();

    QString message;
    in >> message;

    if (!in.commitTransaction()) {
        return;
    }

    if (message == "ServerHello") {
        m_results[m_testIndex] = true;
    }

    m_tcpclient->disconnectFromHost();
    m_testIndex++;
    start();
}

void TestDelegatee::onSocketError()
{
    m_results[m_testIndex] = false;

    m_testIndex++;
    start();
}
