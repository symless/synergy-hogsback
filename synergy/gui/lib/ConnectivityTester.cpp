#include "ConnectivityTester.h"

#include "TestDelegatee.h"
#include "CloudClient.h"
#include "LogManager.h"

#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QMetaType>

ConnectivityTester::ConnectivityTester(QObject* parent) :
    QObject(parent),
    m_localHostname(QHostInfo::localHostName()),
    m_tcpServer(new QTcpServer()),
    m_testThread(NULL),
    m_testCaseBatchSize(0)
{
    m_tcpServer->listen(QHostAddress::Any, kServerPort);
    connect(m_tcpServer, &QTcpServer::newConnection, this,
        &ConnectivityTester::onNewConnection);

    connect(this, &ConnectivityTester::startTesting, this,
        &ConnectivityTester::onStartTesting);

    m_cloudClient = qobject_cast<CloudClient*>(CloudClient::instance());
    connect(m_cloudClient, SIGNAL(receivedScreens(QByteArray)), this,
            SLOT(testNewScreens(QByteArray)));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(pollScreens()));
    m_timer.start(1000);
}

ConnectivityTester::~ConnectivityTester()
{
    delete m_tcpServer;
}

QStringList ConnectivityTester::getSuccessfulResults(int screenId) const
{
    if (m_screenSuccessfulResults.contains(screenId)) {
        return m_screenSuccessfulResults[screenId];
    }
    else {
        return QStringList();
    }
}

void ConnectivityTester::testNewScreens(QByteArray reply)
{
    QJsonDocument doc = QJsonDocument::fromJson(reply);

    if (doc.isNull()) {
        return;
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();

        // we only care about the screen information
        QJsonArray screens = obj["screens"].toArray();
        QSet<int> latestScreenIdSet;
        foreach (QJsonValue const& v, screens) {
            QJsonObject obj = v.toObject();
            // skip inactive screens
            if (!obj["active"].toBool()) {
                continue;
            }

            QString screenName = obj["name"].toString();

            // skip local screen
            if (screenName != m_localHostname) {
                int screenId = obj["id"].toInt();
                latestScreenIdSet.insert(screenId);
                QSet<int>::const_iterator i = m_screenIdSet.find(screenId);
                // if this is a new screen and there is not a connectivity test running already
                if (i == m_screenIdSet.end() && m_testThread == NULL) {
                    // get ip list
                    QString ipList = obj["ipList"].toString();
                    if (ipList.isEmpty()) {
                        continue;
                    }
                    // combine screen id and ip list separated by comma
                    QString testCase = QString::number(screenId);
                    testCase += ',';
                    testCase += ipList;

                    // add connectivity test case
                    m_pendingTestCases.push_back(testCase);
                    m_testCaseBatchSize++;

                    // add into set
                    m_screenIdSet.insert(screenId);
                }
            }
        }

        // start connectivity test
        if (m_testCaseBatchSize > 0 && m_testThread == NULL) {
            emit startTesting();
        }

        // remove inactive screen id
        m_screenIdSet.intersect(latestScreenIdSet);
    }
}

void ConnectivityTester::pollScreens()
{
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

    QByteArray raw;
    in >> raw;

    QString message(raw);

    if (message == "ClientHello") {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out << "ServerHello";
        socket->write(block);
    }
}

void ConnectivityTester::onStartTesting()
{
    if (m_testThread != NULL) {
        return;
    }

    TestDelegatee* delegatee = new TestDelegatee(m_pendingTestCases, m_testCaseBatchSize);

    m_testThread = new QThread();
    qRegisterMetaType<TestDelegatee::TestResults>("TestResults");

    delegatee->moveToThread(m_testThread);
    connect(delegatee, &TestDelegatee::done, this, &ConnectivityTester::onTestDelegateeDone);
    connect(delegatee, &TestDelegatee::done, delegatee, &TestDelegatee::deleteLater);
    connect(delegatee, &QObject::destroyed, m_testThread, &QThread::quit);
    connect(m_testThread, &QThread::finished, m_testThread, &QThread::deleteLater);
    m_testThread->start();

    QMetaObject::invokeMethod(delegatee, "start",
                              Qt::QueuedConnection);
}

void ConnectivityTester::onTestDelegateeDone(QMap<QString, bool> results)
{
    // iterate through all tested cases
    for (int i = 0; i < m_testCaseBatchSize; i++) {
        QString testCase = m_pendingTestCases[i];
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

        // extract successful and failed test results
        for (int i = 0; i < ipList.size(); i++) {
            bool r = results[ipList[i]];

            if (r) {
                successfulIpList.append(ipList[i]);
                LogManager::debug(QString("connectivity test pass: %1").arg(ipList[i]));
            }
            else {
                failedIpList.append(ipList[i]);
            }
        }

        QString successfulIp = successfulIpList.join(',');
        QString failedIp = failedIpList.join(',');

        // report to cloud
        m_cloudClient->report(screenId, successfulIp, failedIp);

        // update connectivity results
        m_screenSuccessfulResults[screenId] = successfulIpList;

        m_pendingTestCases.pop_front();
    }

    m_testThread = NULL;
    m_testCaseBatchSize = 0;
}
