#include "CloudClient.h"

#include "AppConfig.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QDebug>

// https://alpha1.cloud.symless.com/
// http://127.0.0.1:8080/

static const char kAddScreenUrl[] = "https://alpha1.cloud.symless.com/user/screens/join";
static const char kLoginUrl[] = "https://alpha1.cloud.symless.com/login";
static const char kIdentifyUrl[] = "https://alpha1.cloud.symless.com/user/identify";
static const char kscreensUrl[] = "https://alpha1.cloud.symless.com/user/screens";
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

    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(onGetUserIdFinished(QNetworkReply*)));

    QNetworkReply* reply = m_networkManager->get(req);

    connect(
        reply,
        static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                this, &CloudClient::onReplyError);
}

void CloudClient::addScreen(QString name)
{
    QUrl addScreenUrl = QUrl(kAddScreenUrl);
    QNetworkRequest req (addScreenUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    QJsonObject jsonObject;
    QJsonObject screenObject;
    QJsonObject groupObject;
    screenObject.insert("name", name);
    groupObject.insert ("name", "default");
    jsonObject.insert("screen", screenObject);
    jsonObject.insert("group", groupObject);
    QJsonDocument doc(jsonObject);

    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(onAddScreenFinished(QNetworkReply*)));
    m_networkManager->post(req, doc.toJson());
}

void CloudClient::onAddScreenFinished(QNetworkReply* reply)
{
    reply->deleteLater();
    qDebug() << reply->readAll() << "\n";
    disconnect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(onAddScreenFinished(QNetworkReply*)));
}

void CloudClient::getScreens()
{
    QUrl screensUrl = QUrl(kscreensUrl);
    QNetworkRequest req(screensUrl);
    req.setRawHeader("X-Auth-Token", m_appConfig->userToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(onGetScreensFinished(QNetworkReply*)));

    m_networkManager->get(req);
}

void CloudClient::onLoginFinished(QNetworkReply* reply)
{
    disconnect (m_networkManager, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(onLoginFinished(QNetworkReply*)));

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

void CloudClient::onGetScreensFinished(QNetworkReply* reply)
{
    emit receivedScreens(reply->readAll());
    disconnect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this,
               SLOT(onGetScreensFinished(QNetworkReply*)));
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
