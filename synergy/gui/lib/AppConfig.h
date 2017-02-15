#ifndef APPCONFIGURATION_H
#define APPCONFIGURATION_H

#include "LibMacro.h"
#include "DebugLevel.h"

#include <QObject>
#include <QQmlEngine>
#include <QSettings>

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
    Q_INVOKABLE QString userToken();
    void setUserToken(const QString& token);
    Q_INVOKABLE int userId();
    void setUserId(int id);

protected:
	AppConfig();

private:
    QSettings m_settings;
	DebugLevel m_debugLevel;
    QString m_localIp;
    QString m_userToken;
    int m_userId;
    bool m_dragAndDrop;

    static QObject* s_instance;
};

#endif // APPCONFIGURATION_H
