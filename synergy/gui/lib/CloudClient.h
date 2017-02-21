#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include <QNetworkReply>
#include <QTime>
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
    Q_INVOKABLE bool verifyUser();
    Q_INVOKABLE void getUserId(bool initialCall = true);

    Q_INVOKABLE void getScreens();

signals:
    void loginOk();
    void loginFail(QString error);
    void receivedScreens(QByteArray reply);

private slots:
    void onLoginFinished(QNetworkReply* reply);
    void onGetIdentifyFinished(QNetworkReply* reply);
    void onGetUserIdFinished(QNetworkReply* reply);
    void onGetScreensFinished(QNetworkReply* reply);
    void onReplyError(QNetworkReply::NetworkError code);
    void onRetryGetUserId();

private:
    QByteArray m_Data;
    QNetworkAccessManager* m_networkManager;
    AppConfig* m_appConfig;
    QTime m_elapsedTime;

};

#endif // CLOUDCLIENT_H
