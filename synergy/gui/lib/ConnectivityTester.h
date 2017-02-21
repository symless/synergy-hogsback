#ifndef CONNECTIVITYTESTER_H
#define CONNECTIVITYTESTER_H

#include <QQuickItem>
#include <QObject>

#include <QTimer>
#include <QSet>

class CloudClient;

class ConnectivityTester : public QObject
{
    Q_OBJECT
public:
    explicit ConnectivityTester(QObject* parent = 0);

    Q_PROPERTY(CloudClient* cloudClient WRITE setCloudClient)

    void setCloudClient(CloudClient* cloudClient);

signals:
    void cloudClientSet();

private slots:
    void testNewScreens(QByteArray reply);
    void pollScreens();

private:
    QTimer m_timer;
    QSet<int> m_screenIdSet;
    QString m_localHostname;
    CloudClient* m_cloudClient;
};

#endif // CONNECTIVITYTESTER_H
