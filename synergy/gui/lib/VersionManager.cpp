#include "VersionManager.h"

#include "LogManager.h"
#include "Macro.h"

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

VersionManager::VersionManager()
{
    setVersion(STRINGIZE(SYNERGY_VERSION_STRING));
}
