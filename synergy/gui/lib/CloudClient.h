#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class AppConfig;

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
    QByteArray m_Data;
    QNetworkAccessManager* m_networkManager;
    AppConfig* m_appConfig;

};

#endif // CLOUDCLIENT_H
