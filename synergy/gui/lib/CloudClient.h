#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include "LibMacro.h"

#include <QNetworkReply>
#include <QTimer>
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
    Q_INVOKABLE void getLatestVersion();
    Q_INVOKABLE QString serverHostname() const;

    void report(int destId, QString successfulIp, QString failedIp);
    void updateGroupConfig(QJsonDocument& doc);
    void claimServer();
    void updateScreen(const Screen& screen);
    void uploadLogFile(QString filename);

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
    void onUpdateGroupConfigFinished(QNetworkReply* reply);
    void onUserGroupsFinished(QNetworkReply* reply);
    void onUnsubGroupFinished(QNetworkReply* reply);
    void onGetLatestVersionFinished(QNetworkReply* reply);
    void onUploadLogFileFinished(QNetworkReply* reply);
    void onUploadProgress(qint64 done, qint64 total);
    void onReplyError(QNetworkReply::NetworkError code);
    void onRetryGetUserId();

private:
    void syncConfig();
    bool replyHasError(QNetworkReply* reply);

private:
    QByteArray m_Data;
    QNetworkAccessManager* m_networkManager;
    AppConfig* m_appConfig;
    QTimer m_elapsedTimer;
    int64_t m_groupId = -1;
    int64_t m_screenId = -1;

};

#endif // CLOUDCLIENT_H
