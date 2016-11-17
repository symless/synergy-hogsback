#ifndef IPCREADER_H
#define IPCREADER_H

#include "LibMacro.h"

#include <QObject>
#include <QMutex>

class QTcpSocket;

class LIB_SPEC IpcReader : public QObject
{
	Q_OBJECT;

public:
	IpcReader(QTcpSocket* socket);
	virtual ~IpcReader();
	void start();
	void stop();

signals:
	void readLogLine(const QString& text);

private:
	bool readStream(char* buffer, int length);
	int bytesToInt(const char* buffer, int size);

private slots:
	void read();

private:
	QTcpSocket* m_socket;
	QMutex m_mutex;
};

#endif // IPCREADER_H
