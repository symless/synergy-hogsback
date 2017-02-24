#include "ConnectivityTester.h"

#include "TestDelegatee.h"
#include "CloudClient.h"

#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>

ConnectivityTester::ConnectivityTester(QObject* parent) :
    QObject(parent),
    m_localHostname(QHostInfo::localHostName()),
    m_tcpServer(new QTcpServer()),
    m_testThread(NULL)
{
    m_tcpServer->listen(QHostAddress::Any, kServerPort);
    connect(m_tcpServer, &QTcpServer::newConnection, this,
        &ConnectivityTester::onNewConnection);

    connect(this, &ConnectivityTester::startTesting, this,
        &ConnectivityTester::onStartTesting);
}

ConnectivityTester::~ConnectivityTester()
{
    delete m_tcpServer;
}

void ConnectivityTester::setCloudClient(CloudClient* cloudClient)
{
    if (cloudClient == NULL) return;

    m_cloudClient = cloudClient;

    connect(m_cloudClient, SIGNAL(receivedScreens(QByteArray)), this,
            SLOT(testNewScreens(QByteArray)));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(pollScreens()));
    m_timer.start(3000);
}

void ConnectivityTester::testNewScreens(QByteArray reply)
{
    qDebug() << "check new screens";
    QJsonDocument doc = QJsonDocument::fromJson(reply);
    if (!doc.isNull()) {
        if (doc.isObject()) {
            QJsonObject obj = doc.object();

            QJsonArray screens = obj["screens"].toArray();
            QSet<int> latestScreenIdSet;
            foreach (QJsonValue const& v, screens) {
                QJsonObject obj = v.toObject();
                QString screenName = obj["name"].toString();

                if (screenName != m_localHostname) {
                    int screenId = obj["id"].toInt();
                    latestScreenIdSet.insert(screenId);
                    QSet<int>::const_iterator i = m_screenIdSet.find(screenId);
                    if (i == m_screenIdSet.end()) {
                        // new screen detected, get ip list
                        QString ipList = obj["ipList"].toString();
                        // combine screen id and ip list separated by comma
                        QString testCase = QString::number(screenId);
                        testCase += ',';
                        testCase += ipList;

                        // start connectivity test
                        m_pendingTestCases.push_back(testCase);
                        emit startTesting();

                        // add into set
                        m_screenIdSet.insert(screenId);
                    }
                }
            }

            // remove inactive screen id
            m_screenIdSet.intersect(latestScreenIdSet);
        }
    }
}

void ConnectivityTester::pollScreens()
{
     qDebug() << "polling screens";

     m_cloudClient->getScreens();
}

void ConnectivityTester::onNewConnection()
{
    QTcpSocket* newConnection = m_tcpServer->nextPendingConnection();
    connect(newConnection, &QAbstractSocket::disconnected,
        newConnection, &QObject::deleteLater);
    connect(newConnection, &QIODevice::readyRead,
            this, &ConnectivityTester::onConnectionReadyRead);
}

void ConnectivityTester::onConnectionReadyRead()
{
    QTcpSocket* socket = dynamic_cast<QTcpSocket*>(QObject::sender());

    QDataStream in;
    in.setDevice(socket);
    in.setVersion(QDataStream::Qt_5_7);
    in.startTransaction();

    QString message;
    in >> message;

    if (!in.commitTransaction()) {
        return;
    }

    if (message == "ClientHello") {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_7);
        out << "ServerHello";
        socket->write(block);
    }
}

void ConnectivityTester::onStartTesting()
{
    if (m_testThread != NULL) {
        return;
    }

    TestDelegatee* delegatee = new TestDelegatee(m_pendingTestCases.front());

    m_testThread = new QThread();
    connect(delegatee, SIGNAL(done(QList<bool>)), this, SLOT(onTestDelegateeDone(QList<bool>)));
    connect(delegatee, SIGNAL(done(QList<bool>)), m_testThread, SLOT(quit()));
    connect(delegatee, SIGNAL(done(QList<bool>)), delegatee, SLOT(deleteLater()));
    connect(m_testThread, SIGNAL(finished()), m_testThread, SLOT(deleteLater()));
    delegatee->moveToThread(m_testThread);
    m_testThread->start();

    QMetaObject::invokeMethod(delegatee, "start",
                              Qt::QueuedConnection);
}

void ConnectivityTester::onTestDelegateeDone(QList<bool> results)
{
    QString testCase = m_pendingTestCases.front();
    QStringList parts;
    parts = testCase.split(',');
    int screenId = -1;
    QList<QString> ipList;

    if (parts.size() > 1) {
        screenId = parts[0].toInt();
        for (int i = 1; i < parts.size(); i++) {
            ipList.append(parts[i]);
        }
    }

    QStringList successfulIpList;
    QStringList failedIpList;

    for (int i = 0; i < ipList.size(); i++) {
        bool result = false;
        if (i < results.size()) {
            result = results[i];
        }

        if (result) {
            successfulIpList.append(ipList[i]);
        }
        else {
            failedIpList.append(ipList[i]);
        }
    }

    QString successfulIp = successfulIpList.join(',');
    QString failedIp = failedIpList.join(',');

    disconnect(m_cloudClient, SIGNAL(receivedScreens(QByteArray)), this,
            SLOT(testNewScreens(QByteArray)));
    disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(pollScreens()));

    m_cloudClient->report(screenId, successfulIp, failedIp);

    m_pendingTestCases.pop_front();
    m_testThread = NULL;

    if (!m_pendingTestCases.isEmpty()) {
        emit startTesting();
    }
}
