#ifndef APPCONFIGURATION_H
#define APPCONFIGURATION_H

#include "LibMacro.h"
#include "DebugLevelUtil.h"

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

    Q_INVOKABLE int userId();
    Q_INVOKABLE QString userToken();
    Q_INVOKABLE void save();
    Q_INVOKABLE void clearAuth();

	bool dragAndDrop() const;
	DebugLevel debugLevel();
	QString localIp() const;
    void setLocalIp(const QString& localIp);
    void setUserToken(const QString& token);
    void setUserId(int id);
    int profileId() const;
    void setProfileId(int profileId);
    int screenId() const;
    void setScreenId(int screenId);

protected:
    AppConfig();

private:
    QSettings m_settings;
	DebugLevel m_debugLevel;
    QString m_localIp;
    QString m_userToken;
    int m_userId;
    int m_profileId;
    int m_screenId;
    bool m_dragAndDrop;
};

#endif // APPCONFIGURATION_H
