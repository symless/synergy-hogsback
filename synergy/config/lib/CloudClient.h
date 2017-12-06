#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include "LibMacro.h"

#include <QNetworkReply>
#include <QTimer>
#include <QObject>
#include <QQmlEngine>
#include <boost/optional.hpp>

class QNetworkAccessManager;
class QNetworkReply;
class AppConfig;
class UIScreen;

class LIB_SPEC CloudClient : public QObject
{
    Q_OBJECT

public:
    explicit CloudClient(QObject* parent = 0);
    ~CloudClient();

    Q_INVOKABLE void getUserToken();
    Q_INVOKABLE bool verifyUser();
    Q_INVOKABLE void getUserId(bool initialCall = true);
    Q_INVOKABLE void switchProfile(QString profileName = "default");
    Q_INVOKABLE void unsubProfile(int screenId);
    Q_INVOKABLE void userProfiles();
    Q_INVOKABLE void checkUpdate();
    Q_INVOKABLE QString serverHostname() const;
    Q_INVOKABLE QString loginClientId();

    QNetworkRequest newRequest(QUrl url);
    void report(int destId, QString successfulIp, QString failedIp);
    void updateProfileConfig(QJsonDocument& doc);
    void updateScreen(const UIScreen& screen);
    void uploadLogFile(QString source, QString target);
    void receivedScreensInterface(QByteArray msg);
    void enableVersionCheck();

    static QObject* instance(QQmlEngine* engine = NULL, QJSEngine* scriptEngine = NULL);

signals:
    void loginOk();
    void profileUpdated();
    void loginFail(QString error);
    void receivedProfiles(QMap<QString, int>);
    void invalidAuth();
    void uploadLogFileSuccess(QString url);

private slots:
    void onGetIdentifyFinished(QNetworkReply* reply);
    void onGetUserIdFinished(QNetworkReply* reply);
    void onSwitchProfileFinished(QNetworkReply* reply);
    void onUpdateProfileConfigFinished(QNetworkReply* reply);
    void onUserProfilesFinished(QNetworkReply* reply);
    void onUnsubProfileFinished(QNetworkReply* reply);
    void onCheckUpdateFinished(QNetworkReply* reply);
    void onUploadLogFileFinished(QNetworkReply* reply);
    void onUploadProgress(qint64 done, qint64 total);
    void onReplyError(QNetworkReply::NetworkError code);
    void onRetryGetUserId();

private:
    void syncConfig();
    void setUrls();
    bool replyHasError(QNetworkReply* reply);

private:
    QByteArray m_Data;
    QNetworkAccessManager* m_networkManager;
    AppConfig* m_appConfig;
    QTimer m_elapsedTimer;
    int64_t m_profileId = -1;
    int64_t m_screenId = -1;
    std::string m_loginClientId;
    std::string m_cloudUri;
    QUrl m_userProfilesUrl;
    QUrl m_switchProfileUrl;
    QUrl m_unsubProfileUrl;
    QUrl m_identifyUrl;
    QUrl m_updateProfileConfigUrl;
    QUrl m_reportUrl;
    QUrl m_updateScreenUrl;
    QUrl m_checkUpdateUrl;
    bool m_versionCheck = false;

};

#endif // CLOUDCLIENT_H
