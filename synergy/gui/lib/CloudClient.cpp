#include "CloudClient.h"

#include "AppConfig.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QDebug>

// TODO: use cloud url
static const char kLoginUrl[] = "https://alpha1.cloud.symless.com/login";
static const char kIdentifyUrl[] = "https://alpha1.cloud.symless.com/user/identify";
static const int kPollingTimeout = 60000; // 1 minite

CloudClient::CloudClient(QObject* parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_appConfig = qobject_cast<AppConfig*>(AppConfig::instance());
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

    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(onLoginFinished(QNetworkReply*)));

    m_networkManager->post(req, doc.toJson());
}

void CloudClient::getUserToken()
{
    // contact cloud if there is not a user token
    if (m_appConfig->userToken().isEmpty()) {
        QUrl identifyUrl = QUrl(kIdentifyUrl);
        QNetworkRequest req(identifyUrl);

        connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this,
                SLOT(onGetIdentifyFinished(QNetworkReply*)));

        m_networkManager->get(req);
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
        if (m_elapsedTime.elapsed() < kPollingTimeout) {
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

    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(onGetUserIdFinished(QNetworkReply*)));

    QNetworkReply* reply = m_networkManager->get(req);

    connect(
        reply,
        static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
        this, &CloudClient::onReplyError);
}

void CloudClient::onLoginFinished(QNetworkReply* reply)
{
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

    emit loginOk();
}

void CloudClient::onGetIdentifyFinished(QNetworkReply *reply)
{
    m_Data = reply->readAll();
    reply->deleteLater();
    QByteArray token = reply->rawHeader("X-Auth-Token");
    m_appConfig->setUserToken(token);

    disconnect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(onGetIdentifyFinished(QNetworkReply*)));
}

void CloudClient::onGetUserIdFinished(QNetworkReply *reply)
{
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

    disconnect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(onGetUserIdFinished(QNetworkReply*)));

    if (m_appConfig->userId() == -1) {
        // setup a timer to do the request again
        QTimer::singleShot(500, this, SLOT(onRetryGetUserId()));
    }
    else {
        emit loginOk();
    }
}

void CloudClient::onReplyError(QNetworkReply::NetworkError code)
{
    if (code > 0) {
        qDebug() << "Network request error: " << code;
    }
}

void CloudClient::onRetryGetUserId()
{
    getUserId(false);
}
