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
	m_dragAndDrop(true),
	m_debugLevel(kDebug)
{

}

AppConfig::~AppConfig()
{

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
