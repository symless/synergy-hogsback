#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include "LibMacro.h"

#include <QNetworkReply>
#include <QTimer>
#include <QObject>
#include <QQmlEngine>

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
    Q_INVOKABLE void switchProfile(QString profileName = "default");
    Q_INVOKABLE void unsubProfile(int screenId);
    Q_INVOKABLE void getScreens();
    Q_INVOKABLE void userProfiles();
    Q_INVOKABLE void getLatestVersion();
    Q_INVOKABLE QString serverHostname() const;

    void report(int destId, QString successfulIp, QString failedIp);
    void updateProfileConfig(QJsonDocument& doc);
    void claimServer();
    void updateScreen(const Screen& screen);
    void uploadLogFile(QString filename);

    static QObject* instance(QQmlEngine* engine = NULL, QJSEngine* scriptEngine = NULL);

signals:
    void loginOk();
    void loginFail(QString error);
    void receivedScreens(QByteArray reply);
    void receivedProfiles(QMap<QString, int>);
    void invalidAuth();

private slots:
    void onLoginFinished(QNetworkReply* reply);
    void onGetIdentifyFinished(QNetworkReply* reply);
    void onGetUserIdFinished(QNetworkReply* reply);
    void onGetScreensFinished(QNetworkReply* reply);
    void onSwitchProfileFinished(QNetworkReply* reply);
    void onUpdateProfileConfigFinished(QNetworkReply* reply);
    void onUserProfilesFinished(QNetworkReply* reply);
    void onUnsubProfileFinished(QNetworkReply* reply);
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
    int64_t m_profileId = -1;
    int64_t m_screenId = -1;

};

#endif // CLOUDCLIENT_H
