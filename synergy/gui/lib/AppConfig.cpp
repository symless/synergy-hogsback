#include "AppConfig.h"

QObject* AppConfig::s_instance = NULL;

QObject* AppConfig::instance(QQmlEngine* engine,
										QJSEngine* scriptEngine)
{
	Q_UNUSED(engine)
	Q_UNUSED(scriptEngine)

	if (s_instance == NULL) {
		s_instance = new AppConfig();
	}

	return s_instance;
}

AppConfig::AppConfig() :
    m_debugLevel(kDebug),
    m_userId(-1),
    m_dragAndDrop(true)
{

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
