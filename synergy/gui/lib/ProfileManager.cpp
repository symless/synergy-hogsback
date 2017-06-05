#include "ProfileManager.h"

ProfileManager::ProfileManager()
{
    m_cloudClient = qobject_cast<CloudClient*>(CloudClient::instance());
    connect (m_cloudClient, &CloudClient::receivedProfiles, this,
             &ProfileManager::updateProfiles);
}

QObject*
ProfileManager::instance (QQmlEngine* engine, QJSEngine* scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    static ProfileManager s_instance;
    QQmlEngine::setObjectOwnership(&s_instance, QQmlEngine::CppOwnership);

    return &s_instance;
}

void
ProfileManager::updateProfiles (QMap<QString, int> profileMap)
{
    m_listModel.loadFromMap (profileMap);
}
