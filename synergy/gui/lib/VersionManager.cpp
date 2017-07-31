#include "VersionManager.h"
#include "LogManager.h"

#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>

#ifndef SYNERGY_VERSION_STRING
#define SYNERGY_VERSION_STRING "2.0.0-snapshot.b1-0badc0de"
#endif

static const char kNotNeededToUpdate[] = "none";
static const char kOptionalUpdate[] = "optional";
static const char kRequiredUpdate[] = "required";

QObject* VersionManager::instance(QQmlEngine* engine, QJSEngine* scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    static VersionManager s_instance;
    QQmlEngine::setObjectOwnership(&s_instance, QQmlEngine::CppOwnership);

    return &s_instance;
}

VersionManager::~VersionManager()
{

}

void VersionManager::setVersion(const QString v)
{
    m_version = v;
}

void VersionManager::checkUpdate(QJsonDocument& updateReplyDoc)
{
    if (updateReplyDoc.isNull()) {
        return;
    }

    if (updateReplyDoc.isObject()) {
        QJsonObject obj = updateReplyDoc.object();
        QString updateReplyResult = obj["update"].toString();
        if (updateReplyResult == kNotNeededToUpdate) {
            LogManager::debug(QString("current version is update to date"));
        }
        else if (updateReplyResult == kOptionalUpdate) {
            QString latestVersion = obj["latestVersion"].toString();
            m_latestVersion = latestVersion;
            emit newVersionDetected(latestVersion);
        }
        else if (updateReplyResult == kRequiredUpdate) {
            QMessageBox msgBox;
            msgBox.setText("This version of Synergy is not supported anymore. "
                           "Please <a href='https://symless.com/synergy/downloads'>download</a> the latest version.");
            msgBox.exec();

            throw std::runtime_error("required force update");
        }
        else {
            LogManager::debug(QString("unrecognized update reply: %1").arg(updateReplyResult));
        }
    }
}

QString VersionManager::currentVersion() const
{
    return m_version;
}

QString VersionManager::buildVersion() const {
    return SYNERGY_VERSION_STRING;
}

QString VersionManager::latestVersion() const
{
    return m_latestVersion;
}

VersionManager::VersionManager()
{
    setVersion(SYNERGY_VERSION_STRING);
}
