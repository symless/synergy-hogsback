#include "Multicast.h"

#include <QtNetwork>

Multicast::Multicast() :
    m_socket(new QUdpSocket()),
    m_port(0)
{

}

Multicast::~Multicast()
{
    leave();
    delete m_socket;
    m_socket = NULL;
}

QNetworkInterface Multicast::interface() const
{
    return m_socket->multicastInterface();
}

void Multicast::setAddress(QString address)
{
    m_address.setAddress(address);
}

QString Multicast::address() const
{
    return m_address.toString();
}

int Multicast::port() const
{
    return m_port;
}

void Multicast::setPort(int port)
{
    m_port = port;
}

QString Multicast::localIp() const
{
    return m_localIp;
}

void Multicast::setLocalIp(const QString& localIp)
{
    m_localIp = localIp;
}

void Multicast::join()
{
    connect(m_socket, SIGNAL(readyRead()),
        this, SLOT(processDatagrams()));
    m_socket->bind(QHostAddress::AnyIPv4, m_port,
                QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    m_socket->joinMulticastGroup(m_address,
                getNetworkInterfaceByAddress(m_localIp));

    m_socket->setMulticastInterface(
                getNetworkInterfaceByAddress(m_localIp));
    m_socket->setSocketOption(
                QAbstractSocket::MulticastLoopbackOption, QVariant(0));
}

void Multicast::leave()
{
    disconnect(m_socket, SIGNAL(readyRead()),
        this, SLOT(processDatagrams()));
    m_socket->leaveMulticastGroup(m_address,
                    m_socket->multicastInterface());
}

void Multicast::multicast(MulticastMessage& msg)
{
    QByteArray datagram = msg.toByteArray();
    m_socket->writeDatagram(datagram.data(), datagram.size(), m_address,
                            m_port);
}

void Multicast::processDatagrams()
{
    while (m_socket != NULL && m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(datagram.data(), datagram.size());
        const QString message(datagram);

        parseMessage(message);
    }
}

void Multicast::parseMessage(const QString& message)
{
    MulticastMessage msg(message);
    emit receivedMessage(msg);
}

QNetworkInterface Multicast::getNetworkInterfaceByAddress(
                                    const QString& address)
{
    QList<QNetworkInterface> interfaceList(
                                QNetworkInterface::allInterfaces());
    for (int i = 0; i < interfaceList.size(); i++)
    {
        QList<QNetworkAddressEntry> addressEntryList(
                                        interfaceList[i].addressEntries());
        for (int j = 0; j < addressEntryList.size(); j++)
        {
            if (addressEntryList[j].ip().toString() == address) {
                return interfaceList[i];
            }
        }
    }

    return QNetworkInterface();
}
