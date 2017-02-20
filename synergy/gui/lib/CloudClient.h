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
    Q_INVOKABLE void getUserToken();
    Q_INVOKABLE void verifyUser();

signals:
    void loginOk();
    void loginFail();

public slots:
    void onLoginFinished(QNetworkReply* reply);
    void onGetIdentifyFinished(QNetworkReply* reply);

private:
    QByteArray m_Data;
    QNetworkAccessManager* m_networkManager;
    AppConfig* m_appConfig;

};

#endif // CLOUDCLIENT_H
