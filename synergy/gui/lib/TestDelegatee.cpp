#include "TestDelegatee.h"

#include "LogManager.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QDataStream>

const unsigned int kServerPort = 24810;

TestDelegatee::TestDelegatee(QString& testCase, QObject *parent) :
    QObject(parent),
    m_testCase(testCase),
    m_tcpClient(NULL),
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
}

void TestDelegatee::start()
{
    if (m_testIndex >= m_ipList.size()) {
        emit done(m_results);
        return;
    }

    if (!m_tcpClient) {
        m_tcpClient = new QTcpSocket();
        connect(m_tcpClient, &QAbstractSocket::connected, this, &TestDelegatee::onConnected);
        connect(m_tcpClient, &QIODevice::readyRead, this, &TestDelegatee::onReadyRead);
    }

    QHostAddress address(m_ipList[m_testIndex]);
    m_tcpClient->connectToHost(address, kServerPort);

    if (!m_tcpClient->waitForConnected(3000)) {
        onSocketError();
    }
    else {
        typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
        connect(m_tcpClient, static_cast<QAbstractSocketErrorSignal>(&QAbstractSocket::error),
            this, &TestDelegatee::onSocketError);
    }

    return;
}

void TestDelegatee::onConnected()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);
    out << "ClientHello";
    m_tcpClient->write(block);
}

void TestDelegatee::onReadyRead()
{
    QDataStream in;
    in.setDevice(m_tcpClient);
    in.setVersion(QDataStream::Qt_5_5);

    QByteArray raw;
    in >> raw;

    QString message(raw);

    if (message == "ServerHello") {
        m_results[m_testIndex] = true;
        LogManager::debug(QString("%1 pass").arg(m_ipList[m_testIndex]));
    }

    disconnect(m_tcpClient, &QAbstractSocket::connected, this, &TestDelegatee::onConnected);
    disconnect(m_tcpClient, &QIODevice::readyRead, this, &TestDelegatee::onReadyRead);
    typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
    disconnect(m_tcpClient, static_cast<QAbstractSocketErrorSignal>(&QAbstractSocket::error),
        this, &TestDelegatee::onSocketError);
    m_tcpClient->abort();
    in.setDevice(0);
    m_tcpClient->deleteLater();
    m_tcpClient = NULL;

    m_testIndex++;
    start();
}

void TestDelegatee::onSocketError()
{
    m_results[m_testIndex] = false;
    disconnect(m_tcpClient, &QAbstractSocket::connected, this, &TestDelegatee::onConnected);
    disconnect(m_tcpClient, &QIODevice::readyRead, this, &TestDelegatee::onReadyRead);
    typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
    disconnect(m_tcpClient, static_cast<QAbstractSocketErrorSignal>(&QAbstractSocket::error),
        this, &TestDelegatee::onSocketError);
    m_tcpClient->abort();
    m_tcpClient->deleteLater();
    m_tcpClient = NULL;
    m_testIndex++;
    start();
}
