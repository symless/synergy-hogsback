#include "CloudClient.h"

#include "AppConfig.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

// TODO: use cloud url
static const char kLoginUrl[] = "https://alpha1.cloud.symless.com/login";
static const char kIdentifyUrl[] = "https://alpha1.cloud.symless.com/user/identify";

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

void CloudClient::verifyUser()
{
    if (!m_appConfig->userToken().isEmpty() &&
        m_appConfig->userId() != -1) {
        emit loginOk();
    }
    else {
        emit loginRequired();
    }
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
}
