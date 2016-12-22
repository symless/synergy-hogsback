#include "MulticastManager.h"

#include "Multicast.h"
#include "IpManager.h"
#include "AppConfig.h"
#include "LogManager.h"

#include <QtNetwork>

// http://goo.gl/gFouKf
const QString kDefaultGroupAddress = "239.1.1.0";
const QString kUniqueGroupNetwork = "239.1.1.";
const int kDefaultUniqueGroup = 1;
const int kMaximumUniqueGroup = 255;
const int kDefaultMulticastPort = 24800;

QObject* MulticastManager::s_instance = NULL;

QObject* MulticastManager::instance(QQmlEngine* engine,
										QJSEngine* scriptEngine)
{
	Q_UNUSED(engine)
	Q_UNUSED(scriptEngine)

	static MulticastManager s_instance;

	return &s_instance;
}

MulticastManager::MulticastManager() :
	m_defaultMulticast(NULL),
	m_uniqueMulticast(NULL),
	m_appConfig(NULL),
	m_localHostname(QHostInfo::localHostName())
{
	m_appConfig = qobject_cast<AppConfig*>(AppConfig::instance());

	updateLocalIp();
	joinDefaultGroup();
}

void MulticastManager::handleDefaultMulticastMessage(MulticastMessage msg)
{
	emit receivedDefaultGroupMessage(msg);
}

void MulticastManager::handleUniqueMulticastMessage(MulticastMessage msg)
{
	emit receivedUniqueGroupMessage(msg);
}

MulticastManager::~MulticastManager()
{
	leaveDefaultGroup();
	leaveUniqueGroup();
}

bool MulticastManager::joinedUniqueGroup()
{
	return m_uniqueMulticast != NULL;
}

void MulticastManager::joinDefaultGroup()
{
	if (m_defaultMulticast != NULL) {
		leaveDefaultGroup();
	}

	m_defaultMulticast = new Multicast();
	m_defaultMulticast->setAddress(kDefaultGroupAddress);
	m_defaultMulticast->setPort(kDefaultMulticastPort);
	m_defaultMulticast->setLocalIp(m_localIp);
	connect(m_defaultMulticast, SIGNAL(receivedMessage(MulticastMessage)),
		this, SLOT(handleDefaultMulticastMessage(MulticastMessage)));
	m_defaultMulticast->join();
	printMulticastInterfaceInfo(true);
}

void MulticastManager::leaveDefaultGroup()
{
	if (m_defaultMulticast) {
		disconnect(m_defaultMulticast, SIGNAL(receivedMessage(MulticastMessage)),
			this, SLOT(handleDefaultMulticastMessage(MulticastMessage)));
		delete m_defaultMulticast;
		m_defaultMulticast = NULL;
	}

}

void MulticastManager::joinUniqueGroup(int group)
{
	if (m_uniqueMulticast != NULL) {
		leaveUniqueGroup();
	}

	m_uniqueMulticast = new Multicast();
	m_uniqueMulticast->setAddress(getGroupAddress(group));
	m_uniqueMulticast->setPort(kDefaultMulticastPort);
	m_uniqueMulticast->setLocalIp(m_localIp);
	connect(m_uniqueMulticast, SIGNAL(receivedMessage(MulticastMessage)),
		this, SLOT(handleUniqueMulticastMessage(MulticastMessage)));
	m_uniqueMulticast->join();
	printMulticastInterfaceInfo(false);
}

void MulticastManager::leaveUniqueGroup()
{
	if (m_uniqueMulticast) {
		disconnect(m_uniqueMulticast,
			SIGNAL(receivedMessage(MulticastMessage)),
			this, SLOT(handleUniqueMulticastMessage(MulticastMessage)));

		delete m_uniqueMulticast;
		m_uniqueMulticast = NULL;
	}
}


void MulticastManager::multicastDefaultExistence()
{
	MulticastMessage msg;
	msg.m_type = kDefaultExistence;

	m_defaultMulticast->multicast(msg);
}

void MulticastManager::multicastDefaultServerReply(bool active)
{
	MulticastMessage msg;
	msg.m_type = kDefaultReply;
	msg.m_active = active;
	int group = getGroup(m_uniqueMulticast->address());
	if (group == -1) {
		LogManager::error(QString("failed to convert %1 into integer")
							.arg(m_uniqueMulticast->address()));
		return;
	}
	msg.m_uniqueGroup = QString::number(group);
	m_defaultMulticast->multicast(msg);
}

void MulticastManager::multicastUniqueJoin(int processMode)
{
	if (m_uniqueMulticast == NULL) {
		LogManager::warning(QString("failed to multicast unique join on"
								" a non existing unique group"));
		return;
	}

	MulticastMessage msg;
	msg.m_type = kUniqueJoin;
	msg.m_processMode = processMode;
	msg.m_hostname = m_localHostname;

	m_uniqueMulticast->multicast(msg);
}

void MulticastManager::multicastUniqueLeave(int processMode)
{
	if (m_uniqueMulticast == NULL) {
		LogManager::warning(QString("failed to multicast unique leave on"
								" a non existing unique group"));
		return;
	}

	MulticastMessage msg;
	msg.m_type = kUniqueLeave;
	msg.m_processMode = processMode;
	msg.m_hostname = m_localHostname;
	msg.m_ip = m_localIp;

	m_uniqueMulticast->multicast(msg);
}

void MulticastManager::multicastUniqueClaim()
{
	if (m_uniqueMulticast == NULL) {
		LogManager::warning(QString("failed to multicast unique claim on"
								" a non existing unique group"));
		return;
	}

	MulticastMessage msg;
	msg.m_type = kUniqueClaim;
	msg.m_ip = m_localIp;

	m_uniqueMulticast->multicast(msg);
}

void MulticastManager::multicastUniqueConfig(QString& data,
							int latestSerial)
{
	if (m_uniqueMulticast == NULL) {
		LogManager::warning(QString("failed to multicast unique config on"
								" a non existing unique group"));
		return;
	}

	MulticastMessage msg;
	msg.m_type = kUniqueConfig;
	msg.m_configInfo = QString::number(latestSerial) + "," + data;

	m_uniqueMulticast->multicast(msg);
}

void MulticastManager::multicastUniqueConfigDelta(QString& data)
{
	if (m_uniqueMulticast == NULL) {
		LogManager::warning(QString("failed to multicast unique config "
								"delta on a non existing unique group"));
		return;
	}

	MulticastMessage msg;
	msg.m_type = kUniqueConfigDelta;
	msg.m_configInfo = data;

	m_uniqueMulticast->multicast(msg);
}

int MulticastManager::getFirstActiveServerUniqueGroup(
							QMap<int, bool>& replies) const
{
	int group = -1;

	QMap<int, bool>::const_iterator i;
	for (i = replies.constBegin(); i != replies.constEnd(); ++i) {
		if (i.value() == true) {
			group = i.key();
			break;
		}
	}

	return group;
}

int MulticastManager::getNextAvailableUniqueGroup(
							QMap<int, bool>& replies) const
{
	int group = kDefaultUniqueGroup;

	QMap<int, bool>::const_iterator i;
	for (i = replies.constBegin(); i != replies.constEnd(); ++i) {
		if (i.key() == group) {
			group++;
		}
	}

	if (group > kMaximumUniqueGroup) {
		group = -1;
	}

	return group;
}


QString MulticastManager::getGroupAddress(int group)
{
	QString address = kUniqueGroupNetwork + QString::number(group);

	return address;
}

int MulticastManager::getGroup(QString groupAddress)
{
	int i = groupAddress.lastIndexOf('.');
	int r = -1;

	if (i != -1) {
		QString group = groupAddress.right(groupAddress.size() - i - 1);
		r = group.toInt();
	}

	if (r < kDefaultUniqueGroup || r > kMaximumUniqueGroup) {
		r = -1;
	}

	return r;
}

void MulticastManager::updateLocalIp()
{
	if (m_appConfig->localIp().isEmpty()) {
		IpManager ipManager;
		m_localIp = ipManager.getLocalIPAddresses();
	}
	else {
		m_localIp = m_appConfig->localIp();
	}
}

QString MulticastManager::getLocalHostname() const
{
	return m_localHostname;
}

void MulticastManager::setLocalHostname(const QString &localHostname)
{
	m_localHostname = localHostname;
}

void MulticastManager::printMulticastInterfaceInfo(bool defaultGroup)
{
	if (defaultGroup) {
		QNetworkInterface networkInterface(
					m_defaultMulticast->interface());

		LogManager::info(QString("default multicast group: %1 binds to "
								 "network interface: %2 %3")
							.arg(m_defaultMulticast->address())
							.arg(networkInterface.humanReadableName())
							.arg(m_defaultMulticast->localIp()));
	}
	else {
		QNetworkInterface networkInterface(
							m_uniqueMulticast->interface());

		LogManager::info(QString("unique multicast group: %1 binds to "
								 "network interface: %2 %3")
							.arg(m_uniqueMulticast->address())
							.arg(networkInterface.humanReadableName())
							.arg(m_defaultMulticast->localIp()));
	}
}
