#ifndef MULTICASTMESSAGE_H
#define MULTICASTMESSAGE_H

#include "LibMacro.h"

#include <QString>
#include <QObject>

class LIB_SPEC MulticastMessage : public QObject
{
	Q_OBJECT
	Q_ENUMS(Type)

public:
	MulticastMessage();
	MulticastMessage(const MulticastMessage& message);
	MulticastMessage(const QString& message);

	enum Type {
		kDefaultExistence,
		kDefaultReply,
		kUniqueJoin,
		kUniqueLeave,
		kUniqueClaim,
		kUniqueConfig,
		kUniqueConfigDelta,
		kUnknown
	};

	QByteArray toByteArray();
	QString toReadableString();

	bool m_valid;
	int m_type;
	int m_processMode;
	bool m_active;
	bool m_joinGroup;
	QString m_hostname;
	QString m_ip;
	QString m_uniqueGroup;
	QString m_configInfo;

private:
	void parse(const QString& message);
};

#endif // MULTICASTMESSAGE_H
