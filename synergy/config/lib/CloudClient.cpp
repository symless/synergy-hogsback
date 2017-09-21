#include "CloudClient.h"

#include "UIScreen.h"
#include "LogManager.h"
#include "VersionManager.h"
#include "AppConfig.h"
#include "App.h"

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
#include <cxxopts.hpp>

using namespace std;

static const char kLogUploadUrl[] = "https://symless.com/api/client/log";
static const int kPollingTimeout = 60000; // 1 minute

CloudClient::CloudClient(QObject* parent) : QObject(parent)
{
    setUrls();
    m_networkManager = new QNetworkAccessManager(this);
    m_appConfig = qobject_cast<AppConfig*>(AppConfig::instance());
    m_profileId = m_appConfig->profileId();
    m_screenId = m_appConfig->screenId();
}

CloudClient::~CloudClient()
{
    syncConfig();
}

QObject*
CloudClient::instance (QQmlEngine* engine, QJSEngine* scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    static CloudClient s_instance;
    QQmlEngine::setObjectOwnership(&s_instance, QQmlEngine::CppOwnership);

    return &s_instance;
}

void CloudClient::getUserToken()
{
    if (m_appConfig->userToken().isEmpty() ||
            (m_appConfig->userId() == -1)) {
        QNetworkRequest req(m_identifyUrl);

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
        // if user click login as Symless again while trying to poll the
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
            emit loginFail("Login failed. Please try again.");
            return;
        }
    }

    // start polling cloud to see if we have a valid user ID associated
    // with this user token
    QNetworkRequest req(m_identifyUrl);
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

void CloudClient::unsubProfile(int screenId)
{
    if (screenId == -1) {
        LogManager::debug(QString("can not unsub an unknown screen"));
        return;
    }

    QNetworkRequest req (m_unsubProfileUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    QJsonObject jsonObject;
    jsonObject.insert("screen_id", qint64(screenId));
    jsonObject.insert("profile_id", qint64(m_profileId));
    QJsonDocument doc (jsonObject);

    auto reply = m_networkManager->post(req, doc.toJson());

    connect (reply, &QNetworkReply::finished, [this, reply](){
        onUnsubProfileFinished (reply);
    });

}

void CloudClient::onUpdateProfileConfigFinished(QNetworkReply *reply)
{
    if (replyHasError(reply)) {
        return;
    }
}

void CloudClient::onUserProfilesFinished(QNetworkReply *reply)
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

        QJsonArray profiles = obj["profiles"].toArray();
        QMap<QString, int> profileMap;

        foreach (QJsonValue const& v, profiles) {
            QJsonObject obj = v.toObject();
            int profileId = obj["id"].toInt();
            QString profileName = obj["name"].toString();
            LogManager::debug(QString("Profile ID: %1 name: %2").arg(profileId).arg(profileName));
            profileMap[profileName] = profileId;
        }

        emit receivedProfiles(profileMap);
    }
}

void CloudClient::onUnsubProfileFinished(QNetworkReply *reply)
{
    if (replyHasError(reply)) {
        return;
    }
}

void CloudClient::onCheckUpdateFinished(QNetworkReply *reply)
{
    if (replyHasError(reply)) {
        throw std::runtime_error(QString("failed to check update: %1").arg(reply->errorString()).toStdString());
        return;
    }

    m_Data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(m_Data);

    VersionManager* versionManager = qobject_cast<VersionManager*>(VersionManager::instance());
    versionManager->checkUpdate(doc);
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

void CloudClient::switchProfile(QString profileName)
{
    QNetworkRequest req (m_switchProfileUrl);
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

    QJsonObject profileObject;
    profileObject.insert ("name", profileName);
    QJsonObject jsonObject;
    jsonObject.insert("screen", screenObject);
    jsonObject.insert("profile", profileObject);
    QJsonDocument doc(jsonObject);

    auto reply = m_networkManager->post(req, doc.toJson());
    connect (reply, &QNetworkReply::finished, [this, reply]() {
       this->onSwitchProfileFinished (reply);
    });
}

void CloudClient::onSwitchProfileFinished(QNetworkReply* reply)
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
    auto profile = object["profile_id"];
    if (!screen.isDouble() || !profile.isDouble()) {
        return;
    }

    int64_t originalProfileId = m_profileId;

    m_screenId = screen.toInt();
    m_profileId = profile.toInt();

    syncConfig();
    emit profileUpdated();
}

void CloudClient::userProfiles()
{
    if (m_appConfig->userToken().isEmpty()) {
        return;
    }

    QNetworkRequest req(m_userProfilesUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    auto reply = m_networkManager->get(req);
    connect (reply, &QNetworkReply::finished, [this, reply]() {
        onUserProfilesFinished (reply);
    });
}

void CloudClient::checkUpdate()
{
    QNetworkRequest req(m_checkUpdateUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    VersionManager* versionManager = qobject_cast<VersionManager*>(VersionManager::instance());
    QString currentVersion = versionManager->currentVersion();
    QJsonObject jsonObject;
    jsonObject.insert("currentVersion", currentVersion);

    QJsonDocument doc(jsonObject);

    auto reply = m_networkManager->post(req, doc.toJson());
    connect (reply, &QNetworkReply::finished, [this, reply]() {
        onCheckUpdateFinished (reply);
    });
}

void CloudClient::claimServer()
{
    QNetworkRequest req(m_claimServerUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    QJsonObject jsonObject;
    jsonObject.insert("screen_id", qint64(m_screenId));
    jsonObject.insert("profile_id", qint64(m_profileId));
    QJsonDocument doc(jsonObject);

    m_networkManager->post(req, doc.toJson());
}

void CloudClient::updateScreen(const UIScreen& screen)
{
    QNetworkRequest req(m_updateScreenUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    QJsonObject screenObject;
    screenObject.insert("id", screen.id());
    screenObject.insert("name", screen.name());
    screenObject.insert("status", QString::fromStdString(screenStatusToString(screen.status())));

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
    QNetworkRequest req(m_reportUrl);
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

void CloudClient::updateProfileConfig(QJsonDocument& doc)
{
    QNetworkRequest req(m_updateProfileConfigUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    auto reply = m_networkManager->post(req, doc.toJson());
    connect (reply, &QNetworkReply::finished, [this, reply]{
        onUpdateProfileConfigFinished (reply);
    });
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
    m_appConfig->setProfileId(m_profileId);
    m_appConfig->setScreenId(m_screenId);
}

void CloudClient::setUrls()
{
    bool useTestCloud = g_options.count("use-test-cloud");

    if (useTestCloud) {
        m_cloudUri = "https://alpha1.cloud.symless.com";
        m_loginClientId = "4";
    }
    else {
        m_cloudUri = "https://v1.api.cloud.symless.com";
        m_loginClientId = "3";
    }

    m_userProfilesUrl = QString::fromStdString(m_cloudUri + "/user/profiles"); // TODO: change to /user/%1/profiles
    m_switchProfileUrl = QString::fromStdString(m_cloudUri + "/profile/switch"); // TODO: change to POST to /user/%1/screens
    m_unsubProfileUrl = QString::fromStdString(m_cloudUri + "/profile/unsub"); // TODO: change to DELETE /profile/%1/screen/%2
    m_identifyUrl = QString::fromStdString(m_cloudUri + "/user/identify");
    m_updateProfileConfigUrl = QString::fromStdString(m_cloudUri + "/profile/update"); // TODO: change to PUT to /profile/%1
    m_reportUrl = QString::fromStdString(m_cloudUri + "/report"); // TODO: change to POST to /user/connectivity-reports
    m_claimServerUrl = QString::fromStdString(m_cloudUri + "/profile/server/claim"); // TODO: change to POST to /profile/%1/server
    m_updateScreenUrl = QString::fromStdString(m_cloudUri + "/screen/update"); // TODO: change to PUT to /screen/%1
    m_checkUpdateUrl = QString::fromStdString(m_cloudUri + "/update");
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
    return QString::fromStdString(m_cloudUri);
}

QString CloudClient::loginClientId()
{
    return QString::fromStdString(m_loginClientId);
}
