#include "CloudClient.h"

#include "Screen.h"
#include "LogManager.h"
#include "AppConfig.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QHostInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QDebug>
#include <QEventLoop>
#include <QNetworkInterface>

// https://alpha1.cloud.symless.com/
// http://127.0.0.1:8080/
static const char kJoinGroupUrl[] = "https://alpha1.cloud.symless.com/group/join";
static const char kLeaveGroupUrl[] = "https://alpha1.cloud.symless.com/group/leave";
static const char kLoginUrl[] = "https://alpha1.cloud.symless.com/login";
static const char kIdentifyUrl[] = "https://alpha1.cloud.symless.com/user/identify";
static const char kscreensUrl[] = "https://alpha1.cloud.symless.com/group/screens";
static const char kUpdateGroupConfigUrl[] = "https://alpha1.cloud.symless.com/group/update";
static const char kReportUrl[] = "https://alpha1.cloud.symless.com/report";
static const char kClaimServerUrl[] = "https://alpha1.cloud.symless.com/group/server/claim";
static const char kUpdateScreenUrl[] = "https://alpha1.cloud.symless.com/screen/update";
static const int kPollingTimeout = 60000; // 1 minite

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
        if (0 < m_elapsedTime.elapsed() &&
            m_elapsedTime.elapsed() < kPollingTimeout) {
            skip = true;
        }

        m_elapsedTime.restart();

        if (skip) {
            return;
        }
    }
    else {
        if (m_elapsedTime.elapsed() > kPollingTimeout) {
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

void CloudClient::leaveGroup() {
    static const QUrl removeScreenUrl = QUrl(kLeaveGroupUrl);
    QNetworkRequest req (removeScreenUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    QJsonObject jsonObject;
    jsonObject.insert("screen", qint64(m_screenId));
    jsonObject.insert("group", qint64(m_groupId));
    QJsonDocument doc (jsonObject);

    QEventLoop loop;
    auto reply = m_networkManager->post(req, doc.toJson());
    connect (reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    /*connect (reply, &QNetworkReply::finished, [this, reply]() {
       this->onRemoveScreenFinished (reply);
    });*/
}

void CloudClient::unsubGroup()
{

}


void CloudClient::onRemoveScreenFinished(QNetworkReply* reply)
{
    reply->deleteLater();
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        return;
    }

    m_screenId = -1;
    m_groupId = -1;
}

void CloudClient::onUpdateGroupConfigFinished(QNetworkReply *reply)
{
    if (replyHasError(reply)) {
        return;
    }
}

void CloudClient::joinGroup(int64_t groupId)
{
    static const QUrl addScreenUrl = QUrl(kJoinGroupUrl);
    QNetworkRequest req (addScreenUrl);
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
    if (groupId == -1) {
        groupObject.insert ("name", "default");
    }
    else {
        groupObject.insert ("id", groupId);
    }

    QJsonObject jsonObject;
    jsonObject.insert("screen", screenObject);
    jsonObject.insert("group", groupObject);
    QJsonDocument doc(jsonObject);

    auto reply = m_networkManager->post(req, doc.toJson());
    connect (reply, &QNetworkReply::finished, [this, reply]() {
       this->onJoinGroupFinished (reply);
    });
}

void CloudClient::onJoinGroupFinished(QNetworkReply* reply)
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
    auto screen = object["screen"];
    auto group = object["group"];
    if (!screen.isDouble() || !group.isDouble()) {
        return;
    }

    m_screenId = screen.toInt();
    m_groupId = group.toInt();

    syncConfig();
}

void CloudClient::getScreens()
{
    if (m_groupId == -1) {
        LogManager::debug("retry get screens in 3 secs");
        return;
    }

    QUrl screensUrl = QUrl(kscreensUrl);
    QNetworkRequest req(screensUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    QJsonObject reportObject;
    reportObject.insert("groupId", (qint64)m_groupId);

    QJsonDocument doc(reportObject);

    auto reply = m_networkManager->post(req, doc.toJson());
    connect (reply, &QNetworkReply::finished, [this, reply]{
        onGetScreensFinished (reply);
    });
}

void CloudClient::claimServer()
{
    QUrl claimUrl = QUrl(kClaimServerUrl);
    QNetworkRequest req(claimUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    QJsonObject jsonObject;
    jsonObject.insert("screenId", qint64(m_screenId));
    jsonObject.insert("groupId", qint64(m_groupId));
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
            int id = obj["uid"].toInt();
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
            int id = obj["uid"].toInt();
            m_appConfig->setUserId(id);
        }
    }

    if (m_appConfig->userId() == -1) {
        // setup a timer to do the request again
        QTimer::singleShot(500, this, SLOT(onRetryGetUserId()));
    }
    else {
        emit loginOk();
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
