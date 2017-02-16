#include "MulticastMessage.h"

#include "ProcessMode.h"
#include "AppConfig.h"

#include <QStringList>
#include <QMetaEnum>

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
    m_userId = qobject_cast<AppConfig*>(AppConfig::instance())->userId();
}

MulticastMessage::MulticastMessage(const MulticastMessage& message) :
    m_valid(message.m_valid),
    m_type(message.m_type),
    m_processMode(message.m_processMode),
    m_userId(message.m_userId),
    m_active(message.m_active),
    m_joinGroup(message.m_joinGroup),
    m_hostname(message.m_hostname),
    m_ip(message.m_ip),
    m_uniqueGroup(message.m_uniqueGroup),
    m_configInfo(message.m_configInfo)
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
    m_userId = qobject_cast<AppConfig*>(AppConfig::instance())->userId();

    parse(message);
}

QByteArray MulticastMessage::toByteArray()
{
    QString data;

    data += QString::number(m_type);
    data += "," + QString::number(m_userId);

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

QString MulticastMessage::toReadableString()
{
    QString result;

    int index = metaObject()->indexOfEnumerator("Type");
    QMetaEnum metaEnum = metaObject()->enumerator(index);
    QString type =  metaEnum.valueToKey(m_type);

    result += "type: " + type + "\n";
    result += "userID: " + QString::number(m_userId) + "\n";

    if (m_type == kDefaultReply) {
        result += "active: " + QString::number(m_active) + "\n";
        result += "unique group: " + m_uniqueGroup + "\n";
    }
    else if (m_type == kUniqueJoin) {
        result += "process mode: " + QString::number(m_processMode) + "\n";
        result += "hostname: " + m_hostname + "\n";
    }
    else if (m_type == kUniqueLeave) {
        result += "process mode: " + QString::number(m_processMode) + "\n";
        result += "hostname: " + m_hostname + "\n";
        result += "IP: " + m_ip + "\n";
    }
    else if (m_type == kUniqueClaim) {
        result += "IP: " + m_ip + "\n";
    }
    else if (m_type == kUniqueConfig || m_type == kUniqueConfigDelta) {
        result += "config info: " + m_configInfo + "\n";
    }

    return result;
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
    m_userId = elements[1].toInt();

    if (m_type == kDefaultExistence) {
        if (elements.size() != 2) {
            m_valid = false;
            return;
        }
    }
    else if (m_type == kDefaultReply) {
        if (elements.size() != 4) {
            m_valid = false;
            return;
        }

        m_active = elements[2].toInt();
        m_uniqueGroup = elements[3];
    }
    else if (m_type == kUniqueJoin) {
        if (elements.size() != 4) {
            m_valid = false;
            return;
        }

        m_processMode = elements[2].toInt();
        m_hostname = elements[3];
    }
    else if (m_type == kUniqueLeave) {
        if (elements.size() != 5) {
            m_valid = false;
            return;
        }

        m_processMode = elements[2].toInt();
        m_hostname = elements[3];
        m_ip = elements[4];
    }
    else if (m_type == kUniqueClaim) {
        if (elements.size() != 3) {
            m_valid = false;
            return;
        }

        m_ip = elements[2];
    }
    else if (m_type == kUniqueConfig || m_type == kUniqueConfigDelta) {
        // find the second comma
        int i = message.indexOf(',');
        i = message.indexOf(',', i + 1);

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
