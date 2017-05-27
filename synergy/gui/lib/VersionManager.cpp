#include "VersionManager.h"
#include "LogManager.h"

#ifndef SYNERGY_VERSION_STRING
#define SYNERGY_VERSION_STRING "2.0.0-snapshot.b1-0badc0de"
#endif

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

void VersionManager::checkUpdate(const QString& newVersion)
{
    if (m_version.compare(newVersion) != 0) {
        LogManager::debug(QString("A new version detected: %1").arg(newVersion));
        emit newVersionDetected(newVersion);
    }
}

QString VersionManager::buildVersion() const {
    return SYNERGY_VERSION_STRING;
}

VersionManager::VersionManager()
{
    setVersion(SYNERGY_VERSION_STRING);
}
