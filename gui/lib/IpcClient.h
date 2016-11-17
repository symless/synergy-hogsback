#ifndef IPCCLIENT_H
#define IPCCLIENT_H

#include "LibMacro.h"

#include <QQuickItem>
#include <QAbstractSocket>

class QTcpSocket;
class IpcReader;

class LIB_SPEC IpcClient : public QObject
{
	 Q_OBJECT

public:
	IpcClient();
	virtual ~IpcClient();

	void sendCommand(const QString& command, bool elevate);
	void disconnectFromHost();

public slots:
	void retryConnect();

private:
	void connectToHost();
	void intToBytes(int value, char* buffer, int size);

private slots:
	void sendHello();
	void connected();
	void error(QAbstractSocket::SocketError error);
	void handleReadLogLine(const QString& text);

private:
	QTcpSocket* m_socket;
	IpcReader* m_reader;
	bool m_readerStarted;
	bool m_enabled;
};

#endif // IPCCLIENT_H
