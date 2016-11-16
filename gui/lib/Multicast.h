#ifndef MULTICAST_H
#define MULTICAST_H

#include "MulticastMessage.h"

#include <QNetworkInterface>
#include <QObject>

class QUdpSocket;

class Multicast : public QObject
{
	Q_OBJECT

public:
	Multicast();
	~Multicast();

	QNetworkInterface interface() const;
	void setAddress(QString address);
	QString address() const;
	int port() const;
	void setPort(int port);
	QString localIp() const;
	void setLocalIp(const QString& localIp);

	void join();
	void leave();
	void multicast(MulticastMessage& msg);

signals:
	void receivedMessage(MulticastMessage msg);

private slots:
	void processDatagrams();

private:
	void parseMessage(const QString& message);
	QNetworkInterface getNetworkInterfaceByAddress(const QString& address);

private:
	QUdpSocket* m_socket;
	QHostAddress m_address;
	int m_port;
	QString m_localIp;
};

#endif // MULTICAST_H
