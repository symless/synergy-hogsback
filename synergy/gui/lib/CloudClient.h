#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include "LibMacro.h"

#include <QNetworkReply>
#include <QTime>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class AppConfig;
class Screen;

class LIB_SPEC CloudClient : public QObject
{
    Q_OBJECT

public:
    explicit CloudClient(QObject* parent = 0);
    ~CloudClient();

    Q_INVOKABLE void login(QString email, QString password);
    Q_INVOKABLE void getUserToken();
    Q_INVOKABLE bool verifyUser();
    Q_INVOKABLE void getUserId(bool initialCall = true);
    Q_INVOKABLE void joinGroup(QString groupName = "default");
    Q_INVOKABLE void leaveGroup();
    Q_INVOKABLE void unsubGroup();
    Q_INVOKABLE void getScreens();
    Q_INVOKABLE void userGroups();

    void report(int destId, QString successfulIp, QString failedIp);
    void updateGroupConfig(QJsonDocument& doc);
    void claimServer();
    void updateScreen(const Screen& screen);

signals:
    void loginOk();
    void loginFail(QString error);
    void receivedScreens(QByteArray reply);
    void invalidAuth();

private slots:
    void onLoginFinished(QNetworkReply* reply);
    void onGetIdentifyFinished(QNetworkReply* reply);
    void onGetUserIdFinished(QNetworkReply* reply);
    void onGetScreensFinished(QNetworkReply* reply);
    void onJoinGroupFinished(QNetworkReply* reply);
    void onRemoveScreenFinished(QNetworkReply* reply);
    void onUpdateGroupConfigFinished(QNetworkReply* reply);
    void onUserGroupsFinished(QNetworkReply* reply);
    void onReplyError(QNetworkReply::NetworkError code);
    void onRetryGetUserId();

private:
    void syncConfig();
    bool replyHasError(QNetworkReply* reply);

private:
    QByteArray m_Data;
    QNetworkAccessManager* m_networkManager;
    AppConfig* m_appConfig;
    QTime m_elapsedTime;
    int64_t m_groupId = -1;
    int64_t m_screenId = -1;

};

#endif // CLOUDCLIENT_H
