#ifndef APPCONFIGURATION_H
#define APPCONFIGURATION_H

#include "LibMacro.h"
#include "DebugLevel.h"

#include <QObject>
#include <QQmlEngine>

class LIB_SPEC AppConfig : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(AppConfig)

public:
	static QObject* instance(QQmlEngine* engine = NULL, QJSEngine* scriptEngine = NULL);
	~AppConfig();

	bool dragAndDrop() const;
	DebugLevel debugLevel();
	QString localIp() const;
	void setLocalIp(const QString& localIp);

protected:
	AppConfig();

private:
	bool m_dragAndDrop;
	DebugLevel m_debugLevel;
	QString m_localIp;

	static QObject* s_instance;
};

#endif // APPCONFIGURATION_H
