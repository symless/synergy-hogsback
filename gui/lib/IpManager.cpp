#include "IpManager.h"

#include <QStringList>
#include <QtNetwork>

static const QStringList preferredIPAddress(
                QStringList() <<
                "10.1.2" <<
                "192.168." <<
                "172.");

IpManager::IpManager()
{

}

QString IpManager::getLocalIPAddresses()
{
    QStringList addresses;
    foreach (const QHostAddress& address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
            address != QHostAddress(QHostAddress::LocalHost)) {
            addresses.append(address.toString());
        }
    }

    foreach (const QString& preferredIP, preferredIPAddress) {
        foreach (const QString& address, addresses) {
            if (address.startsWith(preferredIP)) {
                return address;
            }
        }
    }

    return "";
}

QStringList IpManager::getAllLocalIPAddresses()
{
    QStringList addresses;
    foreach (const QHostAddress& address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
            address != QHostAddress(QHostAddress::LocalHost)) {
            addresses.append(address.toString());
        }
    }

    return addresses;
}
