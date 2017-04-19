#include "Screen.h"
#include "LogManager.h"
#include "Common.h"

QHash<ScreenStatus, QString> Screen::m_statusImages = {
    {kConnected, "qrc:/res/image/screen-active.png"},
    {kConnecting, "qrc:/res/image/screen-inactive.png"},
    {kDisconnected, "qrc:/res/image/screen-inactive.png"},
    {kInactive, "qrc:/res/image/screen-inactive.png"}
};

Screen::Screen(QString name) :
    m_id(-1),
    m_posX(-1),
    m_posY(-1),
    m_name(name),
    m_status(kInactive),
    m_locked(false)
{
    m_statusImage = m_statusImages[m_status];
}

int Screen::posX() const
{
    return m_posX;
}

int Screen::posY() const
{
    return m_posY;
}

QString Screen::name() const
{
    return m_name;
}

QString Screen::statusImage() const
{
    return m_statusImage;
}

void Screen::setPosX(int const x)
{
    if ((x <= -kScreenIconWidth) || (x >= kDefaultViewWidth)) {
        LogManager::warning (QString("Attempted to set screen position x coordinate "
                                     "outside visible area. x = %1").arg(x));
        return;
    }

    m_posX = x;
}

void Screen::setPosY(int const y)
{
    if ((y <= -kScreenIconHeight) || (y >= kDefaultViewHeight)) {
        LogManager::warning (QString("Attempted to set screen position y coordinate "
                                     "outside visible area. y = %1").arg(y));
        return;
    }
    m_posY = y;
}

void Screen::setName(QString n)
{
    m_name = n;
}

void Screen::setStatus(ScreenStatus s)
{
    m_status = s;
    m_statusImage = m_statusImages[m_status];
}

void Screen::setStatus(QString s)
{
    ScreenStatus status = stringToScreenStatus(s);
    setStatus(status);
}

bool Screen::locked() const
{
    return m_locked;
}

void Screen::setLocked(bool value)
{
    m_locked = value;
}

ScreenStatus Screen::status() const
{
    return m_status;
}

int Screen::id() const
{
    return m_id;
}

void Screen::setId(int id)
{
    m_id = id;
}
