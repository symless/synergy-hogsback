#include "AppConfig.h"

AppConfig AppConfig::s_instance;

QObject* AppConfig::instance(QQmlEngine* engine,
										QJSEngine* scriptEngine)
{
	Q_UNUSED(engine)
	Q_UNUSED(scriptEngine)
    QQmlEngine::setObjectOwnership(&s_instance, QQmlEngine::CppOwnership);
    return &AppConfig::s_instance;
}

AppConfig::AppConfig() :
    m_debugLevel(kDebug),
    m_dragAndDrop(true)
{
    m_userToken = m_settings.value("userToken", "").toString();
    m_userId = m_settings.value("userId", -1).toInt();
}

AppConfig::~AppConfig()
{
    m_settings.setValue("userToken", m_userToken);
    m_settings.setValue("userId", m_userId);
    m_settings.sync();
}

bool AppConfig::dragAndDrop() const
{
	return m_dragAndDrop;
}

DebugLevel AppConfig::debugLevel()
{
	return m_debugLevel;
}

QString AppConfig::localIp() const
{
	return m_localIp;
}

void AppConfig::setLocalIp(const QString& localIp)
{
    m_localIp = localIp;
}

QString AppConfig::userToken()
{
    return m_userToken;
}

void AppConfig::setUserToken(const QString& token)
{
    m_userToken = token;
}

int AppConfig::userId()
{
    return m_userId;
}

void AppConfig::setUserId(int id)
{
    m_userId = id;
}
