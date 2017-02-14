#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class CloudClient : public QObject
{
    Q_OBJECT

public:
    explicit CloudClient(QObject* parent = 0);

    Q_INVOKABLE void login(QString email, QString password);

signals:

public slots:
    void onfinish(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_networkManager;
    QByteArray m_Data;
};

#endif // CLOUDCLIENT_H
