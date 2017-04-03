#include "TestDelegatee.h"

#include "LogManager.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QDataStream>
#include <QTimer>

const unsigned int kServerPort = 24810;

TestDelegatee::TestDelegatee(const QList<QString>& testCases, int batchSize, QObject* parent) :
    QObject(parent)
{
    for (int i = 0; i < batchSize; i++) {
        QString testCase = testCases[i];
        QStringList parts;
        parts = testCase.split(',');

        if (parts.size() > 1) {
            for (int i = 1; i < parts.size(); i++) {
                m_ipList.append(parts[i]);
            }
        }
    }
}

TestDelegatee::~TestDelegatee()
{
    cleanUp();
}

void TestDelegatee::start()
{
    for (int i = 0; i < m_ipList.size(); i++) {
        QTcpSocket* tcpClient = new QTcpSocket();
        connect(tcpClient, &QAbstractSocket::connected, this, &TestDelegatee::onConnected);
        connect(tcpClient, &QIODevice::readyRead, this, &TestDelegatee::onReadyRead);

        m_socketIpMap.insert(tcpClient, m_ipList[i]);
        m_results.insert(m_ipList[i], false);

        QHostAddress address(m_ipList[i]);
        tcpClient->connectToHost(address, kServerPort);
    }

    QTimer::singleShot(3000, this, SLOT(onTestFinish()));

    return;
}

void TestDelegatee::onConnected()
{
    QTcpSocket* socket = dynamic_cast<QTcpSocket*>(QObject::sender());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << "ClientHello";
    socket->write(block);
}

void TestDelegatee::onReadyRead()
{
    QTcpSocket* socket = dynamic_cast<QTcpSocket*>(QObject::sender());

    QDataStream in;
    in.setDevice(socket);

    QByteArray raw;
    in >> raw;

    QString message(raw);

    if (message == "ServerHello") {

        QMap<QTcpSocket*, QString>::const_iterator i = m_socketIpMap.find(socket);
        if (i != m_socketIpMap.constEnd()) {
            QString ip = i.value();
            m_results[ip] = true;
        }
    }

    in.setDevice(0);
}

void TestDelegatee::onTestFinish()
{
    emit done(m_results);
}

void TestDelegatee::cleanUp()
{
    QMap<QTcpSocket*, QString>::const_iterator i = m_socketIpMap.constBegin();
    while (i != m_socketIpMap.constEnd()) {
        QTcpSocket* socket = i.key();
        disconnect(socket, &QAbstractSocket::connected, this, &TestDelegatee::onConnected);
        disconnect(socket, &QIODevice::readyRead, this, &TestDelegatee::onReadyRead);

        socket->abort();
        socket->deleteLater();
    }
}
