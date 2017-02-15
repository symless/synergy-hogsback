#include "CloudClient.h"

#include "AppConfig.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

// TODO: use cloud url
static const char kCloudUrl[] = "http://192.168.3.59:8080/login";

CloudClient::CloudClient(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);

    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(onfinish(QNetworkReply*)));

    m_appConfig = qobject_cast<AppConfig*>(AppConfig::instance());
}

void CloudClient::login(QString email, QString password)
{
    QUrl cloudUrl = QUrl(kCloudUrl);
    QNetworkRequest req(cloudUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));
    QJsonObject jsonObject;
    jsonObject.insert("email", email);
    jsonObject.insert("password", password);
    QJsonDocument doc(jsonObject);
    m_networkManager->post(req, doc.toJson());
}

void CloudClient::onfinish(QNetworkReply* reply)
{
    m_Data = reply->readAll();
    reply->deleteLater();
    QByteArray token = reply->rawHeader("X-Auth-Token");
    m_appConfig->setUserToken(token);

    QJsonDocument doc = QJsonDocument::fromJson(m_Data);
    QJsonObject obj = doc.object();
    int id = obj["uid"].toInt();
    m_appConfig->setUserId(id);
    qDebug() << token << endl;
}
