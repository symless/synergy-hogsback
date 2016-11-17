#include "IpcClient.h"

#include "IpcReader.h"
#include "Ipc.h"
#include "LogManager.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <iostream>
#include <QTimer>
#include <QDataStream>

IpcClient::IpcClient() :
	m_socket(NULL),
	m_reader(NULL),
	m_readerStarted(false),
	m_enabled(false)
{
#if defined(Q_OS_WIN)
	m_socket = new QTcpSocket(this);
	connect(m_socket, SIGNAL(connected()), this, SLOT(connected()));
	connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));

	m_reader = new IpcReader(m_socket);
	connect(m_reader, SIGNAL(readLogLine(const QString&)), this, SLOT(handleReadLogLine(const QString&)));

	connectToHost();
#endif
}

IpcClient::~IpcClient()
{
	if (m_socket != NULL) {
		delete m_socket;
	}

	if (m_reader != NULL) {
		delete m_reader;
	}
}

void IpcClient::connected()
{
	sendHello();
	LogManager::info("connection established");
}

void IpcClient::connectToHost()
{
	m_enabled = true;

	LogManager::info("connecting to service...");
	m_socket->connectToHost(QHostAddress(QHostAddress::LocalHost), IPC_PORT);

	if (!m_readerStarted) {
		m_reader->start();
		m_readerStarted = true;
	}
}

void IpcClient::disconnectFromHost()
{
#if defined(Q_OS_WIN)
	LogManager::info("service disconnect");
	m_reader->stop();
	m_socket->close();
#endif
}

void IpcClient::error(QAbstractSocket::SocketError error)
{
	QString text;
	switch (error) {
		case 0: text = "connection refused"; break;
		case 1: text = "remote host closed"; break;
		default: text = QString("code=%1").arg(error); break;
	}

	LogManager::error(QString("ipc connection error, %1").arg(text));

	QTimer::singleShot(1000, this, SLOT(retryConnect()));
}

void IpcClient::retryConnect()
{
	if (m_enabled) {
		connectToHost();
	}
}

void IpcClient::sendHello()
{
	QDataStream stream(m_socket);
	stream.writeRawData(kIpcMsgHello, 4);

	char typeBuf[1];
	typeBuf[0] = kIpcClientGui;
	stream.writeRawData(typeBuf, 1);
}

void IpcClient::sendCommand(const QString& command, bool elevate)
{
#if defined(Q_OS_WIN)
	QDataStream stream(m_socket);

	stream.writeRawData(kIpcMsgCommand, 4);

	std::string stdStringCommand = command.toStdString();
	const char* charCommand = stdStringCommand.c_str();
	int length = strlen(charCommand);

	char lenBuf[4];
	intToBytes(length, lenBuf, 4);
	stream.writeRawData(lenBuf, 4);
	stream.writeRawData(charCommand, length);

	char elevateBuf[1];
	elevateBuf[0] = elevate ? 1 : 0;
	stream.writeRawData(elevateBuf, 1);
#endif
}

void IpcClient::handleReadLogLine(const QString& text)
{
	LogManager::row(text);
}

void IpcClient::intToBytes(int value, char* buffer, int size)
{
	if (size == 1) {
		buffer[0] = value & 0xff;
	}
	else if (size == 2) {
		buffer[0] = (value >> 8) & 0xff;
		buffer[1] = value & 0xff;
	}
	else if (size == 4) {
		buffer[0] = (value >> 24) & 0xff;
		buffer[1] = (value >> 16) & 0xff;
		buffer[2] = (value >> 8) & 0xff;
		buffer[3] = value & 0xff;
	}
}
