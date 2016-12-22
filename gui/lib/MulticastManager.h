#ifndef MULTICASTMANAGER_H
#define MULTICASTMANAGER_H

#include "LibMacro.h"
#include "MulticastMessage.h"

#include <QHostAddress>
#include <QNetworkInterface>
#include <QQmlEngine>

class AppConfig;
class Multicast;

class LIB_SPEC MulticastManager : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(MulticastManager)

public:
	static QObject* instance(QQmlEngine* engine = NULL, QJSEngine* scriptEngine = NULL);
	~MulticastManager();

	bool joinedUniqueGroup();
	void joinDefaultGroup();
	void leaveDefaultGroup();
	void joinUniqueGroup(int group);
	void leaveUniqueGroup();

	void multicastDefaultExistence();
	void multicastDefaultServerReply(bool active);
	void multicastUniqueJoin(int processMode);
	void multicastUniqueLeave(int processMode);
	void multicastUniqueClaim();
	void multicastUniqueConfig(QString& data, int latestSerial);
	void multicastUniqueConfigDelta(QString& data);

	int getFirstActiveServerUniqueGroup(QMap<int, bool>& replies) const;
	int getNextAvailableUniqueGroup(QMap<int, bool>& replies) const;

	QString getLocalHostname() const;
	void setLocalHostname(const QString &localHostname);

protected:
	MulticastManager();

signals:
	void receivedDefaultGroupMessage(MulticastMessage msg);
	void receivedUniqueGroupMessage(MulticastMessage msg);

private slots:
	void handleDefaultMulticastMessage(MulticastMessage msg);
	void handleUniqueMulticastMessage(MulticastMessage msg);

private:
	void printMulticastInterfaceInfo(bool defaultGroup);
	QString getGroupAddress(int group);
	int getGroup(QString groupAddress);
	void updateLocalIp();

private:
	Multicast* m_defaultMulticast;
	Multicast* m_uniqueMulticast;
	AppConfig* m_appConfig;
	QString m_localHostname;
	QString m_localIp;

	static QObject* s_instance;
};

#endif // MULTICASTMANAGER_H
