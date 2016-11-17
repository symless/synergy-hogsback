#include "MulticastMessage.h"

#include "ProcessMode.h"

#include <QStringList>

const QString kSeperator = ",";

MulticastMessage::MulticastMessage() :
	m_valid(true),
	m_type(kUnknown),
	m_processMode(kUnknownMode),
	m_active(true),
	m_joinGroup(false),
	m_hostname(QString()),
	m_ip(QString()),
	m_uniqueGroup(QString()),
	m_configInfo(QString())
{
}

MulticastMessage::MulticastMessage(const QString& message) :
	m_valid(true),
	m_type(kUnknown),
	m_processMode(kUnknownMode),
	m_active(true),
	m_joinGroup(false),
	m_hostname(QString()),
	m_ip(QString()),
	m_uniqueGroup(QString()),
	m_configInfo(QString())
{
	parse(message);
}

QByteArray MulticastMessage::toByteArray()
{
	QString data;

	data += QString::number(m_type);

	if (m_type == kDefaultReply) {
		data += "," + QString::number(m_active);
		data += "," + m_uniqueGroup;
	}
	else if (m_type == kUniqueJoin) {
		data += "," + QString::number(m_processMode);
		data += "," + m_hostname;
	}
	else if (m_type == kUniqueLeave) {
		data += "," + QString::number(m_processMode);
		data += "," + m_hostname;
		data += "," + m_ip;
	}
	else if (m_type == kUniqueClaim) {
		data += "," + m_ip;
	}
	else if (m_type == kUniqueConfig || m_type == kUniqueConfigDelta) {
		data += "," + m_configInfo;
	}

	return data.toUtf8();
}

void MulticastMessage::parse(const QString& message)
{
	if (message.size() == 0) {
		m_valid = false;
		return;
	}

	QStringList elements;
	elements = message.split(kSeperator);

	if (elements.empty()) {
		m_valid = false;
		return;
	}

	m_type = elements[0].toInt();

	if (m_type == kDefaultExistence) {
		if (elements.size() != 1) {
			m_valid = false;
			return;
		}
	}
	else if (m_type == kDefaultReply) {
		if (elements.size() != 3) {
			m_valid = false;
			return;
		}

		m_active = elements[1].toInt();
		m_uniqueGroup = elements[2];
	}
	else if (m_type == kUniqueJoin) {
		if (elements.size() != 3) {
			m_valid = false;
			return;
		}

		m_processMode = elements[1].toInt();
		m_hostname = elements[2];
	}
	else if (m_type == kUniqueLeave) {
		if (elements.size() != 4) {
			m_valid = false;
			return;
		}

		m_processMode = elements[1].toInt();
		m_hostname = elements[2];
		m_ip = elements[3];
	}
	else if (m_type == kUniqueClaim) {
		if (elements.size() != 2) {
			m_valid = false;
			return;
		}

		m_ip = elements[1];
	}
	else if (m_type == kUniqueConfig || m_type == kUniqueConfigDelta) {
		int i = message.indexOf(',');

		if (i == -1 || i >= message.size()) {
			m_valid = false;
			return;
		}

		m_configInfo = message.right(message.size() - i - 1);
	}
	else {
		m_valid = false;
	}
}
