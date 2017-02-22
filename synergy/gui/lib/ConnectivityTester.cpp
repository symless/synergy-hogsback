#include "ConnectivityTester.h"

#include "CloudClient.h"

#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ConnectivityTester::ConnectivityTester(QObject* parent) :
    QObject(parent),
    m_localHostname(QHostInfo::localHostName())
{
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

                        // start connectivity test
                        qDebug() << "start testing connectivity with " << ipList;

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
