#include "CloudClient.h"

#include "Screen.h"
#include "LogManager.h"
#include "VersionManager.h"
#include "AppConfig.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QHostInfo>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QDebug>
#include <QEventLoop>
#include <QHttpMultiPart>
#include <QNetworkInterface>

#ifdef SYNERGY_DEVELOPER_MODE
#define SYNERGY_CLOUD_URI "http://127.0.0.1:8080"
#else
#define SYNERGY_CLOUD_URI "https://v1.api.cloud.symless.com"
#endif

static const char kUserGroupsUrl[] = SYNERGY_CLOUD_URI "/user/profiles";
static const char kSwitchGroupUrl[] = SYNERGY_CLOUD_URI "/profile/switch";
static const char kUnsubGroupUrl[] = SYNERGY_CLOUD_URI "/profile/unsub";
static const char kLoginUrl[] = SYNERGY_CLOUD_URI "/login";
static const char kIdentifyUrl[] = SYNERGY_CLOUD_URI "/user/identify";
static const char kGroupScreensUrl[] = SYNERGY_CLOUD_URI "/profile/%1/screens";
static const char kUpdateGroupConfigUrl[] = SYNERGY_CLOUD_URI "/profile/update";
static const char kReportUrl[] = SYNERGY_CLOUD_URI "/report";
static const char kClaimServerUrl[] = SYNERGY_CLOUD_URI "/profile/server/claim";
static const char kUpdateScreenUrl[] = SYNERGY_CLOUD_URI "/screen/update";
static const char kLatestVersionUrl[] = SYNERGY_CLOUD_URI "/version";
static const char kLogUploadUrl[] = "https://symless.com/api/client/log";
static const int kPollingTimeout = 60000; // 1 minute

CloudClient::CloudClient(QObject* parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_appConfig = qobject_cast<AppConfig*>(AppConfig::instance());
    m_groupId = m_appConfig->groupId();
    m_screenId = m_appConfig->screenId();
}

CloudClient::~CloudClient()
{
    syncConfig();
}

void CloudClient::login(QString email, QString password)
{
    QUrl cloudUrl = QUrl(kLoginUrl);
    QNetworkRequest req(cloudUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    QJsonObject jsonObject;
    jsonObject.insert("email", email);
    jsonObject.insert("password", password);
    QJsonDocument doc(jsonObject);

    auto reply = m_networkManager->post(req, doc.toJson());
    connect (reply, &QNetworkReply::finished, [this, reply]() {
        onLoginFinished (reply);
    });
}

void CloudClient::getUserToken()
{
    // contact cloud if there is not a user token
    if (m_appConfig->userToken().isEmpty()) {
        QUrl identifyUrl = QUrl(kIdentifyUrl);
        QNetworkRequest req(identifyUrl);

        auto reply = m_networkManager->get(req);
        connect (reply, &QNetworkReply::finished, [this, reply]() {
            onGetIdentifyFinished (reply);
        });
    }
}

bool CloudClient::verifyUser()
{
    if (!m_appConfig->userToken().isEmpty() &&
        m_appConfig->userId() != -1) {
        return true;
    }

    return false;
}

void CloudClient::getUserId(bool initialCall)
{
    if (initialCall) {
        // if user click login as google again while trying to poll the
        // user Id, we only need to restart the time and skip creating
        // a new poll as the last one probably is not finished yet
        bool skip = false;
        if (m_elapsedTimer.remainingTime() > 0) {
            skip = true;
        }

        m_elapsedTimer.start(kPollingTimeout);

        if (skip) {
            return;
        }
    }
    else {
        if (m_elapsedTimer.remainingTime() <= 0) {
            emit loginFail("Failed to use Google to login. Please try again.");
            return;
        }
    }

    // start polling cloud to see if we have a valid user ID associated
    // with this user token
    QUrl identifyUrl = QUrl(kIdentifyUrl);
    QNetworkRequest req(identifyUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    auto reply = m_networkManager->get(req);
    connect (reply, &QNetworkReply::finished, [this, reply](){
        onGetUserIdFinished (reply);
    });

    connect(
        reply,
        static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                this, &CloudClient::onReplyError);
}

void CloudClient::unsubGroup()
{
    static const QUrl unsubGroupUrl = QUrl(kUnsubGroupUrl);
    QNetworkRequest req (unsubGroupUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    QJsonObject jsonObject;
    jsonObject.insert("screen_id", qint64(m_screenId));
    jsonObject.insert("group_id", qint64(m_groupId));
    QJsonDocument doc (jsonObject);

    auto reply = m_networkManager->post(req, doc.toJson());

    connect (reply, &QNetworkReply::finished, [this, reply](){
        onUnsubGroupFinished (reply);
    });

}

void CloudClient::onUpdateGroupConfigFinished(QNetworkReply *reply)
{
    if (replyHasError(reply)) {
        return;
    }
}

void CloudClient::onUserGroupsFinished(QNetworkReply *reply)
{
    if (replyHasError(reply)) {
        return;
    }

    m_Data = reply->readAll();
    reply->deleteLater();
    QByteArray token = reply->rawHeader("X-Auth-Token");
    m_appConfig->setUserToken(token);

    QJsonDocument doc = QJsonDocument::fromJson(m_Data);

    if (doc.isNull()) {
        return;
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();

        QJsonArray groups = obj["profiles"].toArray();

        foreach (QJsonValue const& v, groups) {
            QJsonObject obj = v.toObject();
            int groupId = obj["id"].toInt();
            QString groupName = obj["name"].toString();

            LogManager::debug(QString("group ID: %1 name: %2").arg(groupId).arg(groupName));
        }
    }
}

void CloudClient::onUnsubGroupFinished(QNetworkReply *reply)
{
    if (replyHasError(reply)) {
        return;
    }

    m_groupId = -1;
    syncConfig();
}

void CloudClient::onGetLatestVersionFinished(QNetworkReply *reply)
{
    if (replyHasError(reply)) {
        return;
    }

    m_Data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(m_Data);

    if (doc.isNull()) {
        return;
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        QString version = obj["latestVersion"].toString();
        VersionManager* versionManager = qobject_cast<VersionManager*>(VersionManager::instance());
        versionManager->checkUpdate(version);
    }
}

void CloudClient::onUploadLogFileFinished(QNetworkReply *reply)
{
    if (replyHasError(reply)) {
        LogManager::error(reply->errorString());
        return;
    }

    m_Data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(m_Data);

    if (doc.isNull()) {
        return;
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        bool success = obj["success"].toBool();

        if (success) {
            QString msg = obj["message"].toString();
            LogManager::info(msg);
        }
    }
}

void CloudClient::onUploadProgress(qint64 done, qint64 total)
{
    // TODO: Show progress in console
}

void CloudClient::switchGroup(QString groupName)
{
    static const QUrl switchGroupUrl = QUrl(kSwitchGroupUrl);
    QNetworkRequest req (switchGroupUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    QStringList ipList;

    foreach (QHostAddress const& address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
                address != QHostAddress(QHostAddress::LocalHost)) {
            ipList.push_back(address.toString());
        }
    }

    QJsonObject screenObject;

    screenObject.insert("name", QHostInfo::localHostName());
    screenObject.insert("ipList", ipList.join(","));
    screenObject.insert("status", "Disconnected");

    QJsonObject groupObject;
    groupObject.insert ("name", groupName);
    QJsonObject jsonObject;
    jsonObject.insert("screen", screenObject);
    jsonObject.insert("profile", groupObject);
    QJsonDocument doc(jsonObject);

    auto reply = m_networkManager->post(req, doc.toJson());
    connect (reply, &QNetworkReply::finished, [this, reply]() {
       this->onSwitchGroupFinished (reply);
    });
}

void CloudClient::onSwitchGroupFinished(QNetworkReply* reply)
{
    reply->deleteLater();
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        return;
    }

    auto doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isObject()) {
        return;
    }

    auto object = doc.object();
    auto screen = object["screen_id"];
    auto group = object["profile_id"];
    if (!screen.isDouble() || !group.isDouble()) {
        return;
    }

    int64_t originalGroupId = m_groupId;

    m_screenId = screen.toInt();
    m_groupId = group.toInt();

    // HACK: Remove this hack after we have profile finished
    if (m_groupId != originalGroupId) {
        claimServer();
    }

    syncConfig();
}

void CloudClient::getScreens()
{
    if (m_groupId < 0) {
        LogManager::debug("retry get screens in 3 secs");
        return;
    }

    QUrl groupScreensUrl = QUrl(QString(kGroupScreensUrl).arg(QString::number(m_groupId)));
    QNetworkRequest req(groupScreensUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());

    auto reply = m_networkManager->get(req);
    connect (reply, &QNetworkReply::finished, [this, reply]{
        onGetScreensFinished (reply);
    });
}

void CloudClient::userGroups()
{
    if (m_appConfig->userToken().isEmpty()) {
        return;
    }

    QUrl userGroupsUrl = QUrl(kUserGroupsUrl);
    QNetworkRequest req(userGroupsUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    auto reply = m_networkManager->get(req);
    connect (reply, &QNetworkReply::finished, [this, reply]() {
        onUserGroupsFinished (reply);
    });
}

void CloudClient::getLatestVersion()
{
    QUrl latestVersionUrl = QUrl(kLatestVersionUrl);
    QNetworkRequest req(latestVersionUrl);

    auto reply = m_networkManager->get(req);
    connect (reply, &QNetworkReply::finished, [this, reply]() {
        onGetLatestVersionFinished (reply);
    });
}

void CloudClient::claimServer()
{
    QUrl claimUrl = QUrl(kClaimServerUrl);
    QNetworkRequest req(claimUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    QJsonObject jsonObject;
    jsonObject.insert("screen_id", qint64(m_screenId));
    jsonObject.insert("profile_id", qint64(m_groupId));
    QJsonDocument doc(jsonObject);

    m_networkManager->post(req, doc.toJson());

}

void CloudClient::updateScreen(const Screen& screen)
{
    QUrl claimUrl = QUrl(kUpdateScreenUrl);
    QNetworkRequest req(claimUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    QJsonObject screenObject;
    screenObject.insert("id", screen.id());
    screenObject.insert("name", screen.name());
    screenObject.insert("status", screenStatusToString(screen.status()));

    QJsonDocument doc(screenObject);

    m_networkManager->post(req, doc.toJson());
}

void CloudClient::uploadLogFile(QString filename)
{
    QFileInfo fileInfo(filename);
    QString withoutPath(fileInfo.fileName());
    QString withoutExt(withoutPath.section('.',0, 0));
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart idPart;
    idPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"logId\""));
    idPart.setBody(withoutExt.toUtf8());

    QHttpPart logPart;
    logPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"log\"; filename=\"" + withoutPath + "\""));
    QFile *file = new QFile(filename);
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    logPart.setBodyDevice(file);
    // delete file with the multiPart
    file->setParent(multiPart);

    multiPart->append(idPart);
    multiPart->append(logPart);

    QUrl logUploadUrl = QUrl(kLogUploadUrl);
    QNetworkRequest req(logUploadUrl);

    auto reply = m_networkManager->post(req, multiPart);
    // delete the multiPart with the reply
    multiPart->setParent(reply);

    connect (reply, &QNetworkReply::finished, [this, reply]{
        onUploadLogFileFinished (reply);
    });
    connect(reply, SIGNAL(uploadProgress(qint64, qint64)),
          this, SLOT  (onUploadProgress(qint64, qint64)));
}

void CloudClient::report(int destId, QString successfulIpList, QString failedIpList)
{
    QUrl reportUrl = QUrl(kReportUrl);
    QNetworkRequest req(reportUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    QJsonObject reportObject;
    reportObject.insert("src", (qint64)m_screenId);
    reportObject.insert("dest", destId);
    reportObject.insert("successfulIpList", successfulIpList);
    reportObject.insert("failedIpList", failedIpList);
    QJsonDocument doc(reportObject);

    m_networkManager->post(req, doc.toJson());

    LogManager::debug(QString("report to cloud: destId %1 successfulIp %2 failedIp %3").arg(destId).arg(successfulIpList).arg(failedIpList));
}

void CloudClient::updateGroupConfig(QJsonDocument& doc)
{
    QUrl reportUrl = QUrl(kUpdateGroupConfigUrl);
    QNetworkRequest req(reportUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    auto reply = m_networkManager->post(req, doc.toJson());
    connect (reply, &QNetworkReply::finished, [this, reply]{
        onUpdateGroupConfigFinished (reply);
    });
}

void CloudClient::onLoginFinished(QNetworkReply* reply)
{
    if (replyHasError(reply)) {
        emit loginFail(reply->errorString());
        return;
    }

    m_Data = reply->readAll();
    reply->deleteLater();
    QByteArray token = reply->rawHeader("X-Auth-Token");
    m_appConfig->setUserToken(token);

    QJsonDocument doc = QJsonDocument::fromJson(m_Data);
    if (!doc.isNull()) {
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            int id = obj["user"].toInt();
            m_appConfig->setUserId(id);
            emit loginOk();
            return;
        }
    }

    emit loginFail(reply->errorString());
}

void CloudClient::onGetIdentifyFinished(QNetworkReply *reply)
{
    if (replyHasError(reply)) {
        return;
    }

    m_Data = reply->readAll();
    reply->deleteLater();
    QByteArray token = reply->rawHeader("X-Auth-Token");
    m_appConfig->setUserToken(token);
}

void CloudClient::onGetUserIdFinished(QNetworkReply *reply)
{
    if (replyHasError(reply)) {
        return;
    }

    m_Data = reply->readAll();
    reply->deleteLater();
    QByteArray token = reply->rawHeader("X-Auth-Token");
    m_appConfig->setUserToken(token);

    QJsonDocument doc = QJsonDocument::fromJson(m_Data);
    if (!doc.isNull()) {
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            int id = obj["user"].toInt();
            m_appConfig->setUserId(id);
        }
    }

    if (m_appConfig->userId() == -1) {
        // setup a timer to do the request again
        QTimer::singleShot(500, this, SLOT(onRetryGetUserId()));
    }
    else {
        emit loginOk();
        m_elapsedTimer.stop();
    }
}

void CloudClient::onGetScreensFinished(QNetworkReply* reply)
{
    if (replyHasError(reply)) {
        return;
    }

    reply->deleteLater();
    emit receivedScreens (reply->readAll());
}

void CloudClient::onReplyError(QNetworkReply::NetworkError code)
{
    if (code > 0) {
        LogManager::error(QString("Network request error: %1").arg(code));
    }
}

void CloudClient::onRetryGetUserId()
{
    getUserId(false);
}

void CloudClient::syncConfig()
{
    m_appConfig->setGroupId(m_groupId);
    m_appConfig->setScreenId(m_screenId);
}

bool CloudClient::replyHasError(QNetworkReply* reply)
{
    bool result = false;
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if (statusCode != 200) {
        LogManager::error(QString("Reply status code: %1").arg(statusCode.toInt()));
        m_Data = reply->readAll();
        reply->deleteLater();

        QJsonDocument doc = QJsonDocument::fromJson(m_Data);
        if (!doc.isNull()) {
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                QString errorMsg = obj["error"].toString();
                LogManager::error(QString("Reply error message: %1").arg(errorMsg));
            }
        }

        result = true;
    }

    // access denied, e.g. bad auth token
    if (statusCode == 403) {
        emit invalidAuth();
    }

    return result;
}


QString
CloudClient::serverHostname() const {
    return SYNERGY_CLOUD_URI;
}
