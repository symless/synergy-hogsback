#include "AppConfig.h"

QObject* AppConfig::instance(QQmlEngine* engine,
										QJSEngine* scriptEngine)
{
	Q_UNUSED(engine)
	Q_UNUSED(scriptEngine)

    static AppConfig s_instance;
    QQmlEngine::setObjectOwnership(&s_instance, QQmlEngine::CppOwnership);

    return &s_instance;
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
    save();
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

void AppConfig::save()
{
    m_settings.setValue("userToken", m_userToken);
    m_settings.setValue("userId", m_userId);
    m_settings.sync();
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
