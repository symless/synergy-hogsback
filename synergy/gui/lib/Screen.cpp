#include "Screen.h"
#include "LogManager.h"
#include "Common.h"

QHash<ScreenState, QString> Screen::m_stateImages;

Screen::Screen(QString name) :
    m_id(-1),
    m_posX(-1),
    m_posY(-1),
    m_name(name),
    m_state(kDisconnected),
    m_locked(false)
{
    m_stateImages[kConnected] = "qrc:/res/image/screen-active.png";
    m_stateImages[kConnecting] = "qrc:/res/image/screen_icon_running.png";
    m_stateImages[kDisconnected] = "qrc:/res/image/screen-active.png";
    m_stateImages[kInactive] = "qrc:/res/image/screen-inactive.png";

    m_stateImage = m_stateImages[m_state];
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

ScreenState Screen::state() const
{
    return m_state;
}

QString Screen::stateImage() const
{
    return m_stateImage;
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

void Screen::setState(ScreenState s)
{
    m_state = s;
    m_stateImage = m_stateImages[m_state];
}

bool Screen::locked() const
{
    return m_locked;
}

void Screen::setLocked(bool value)
{
    m_locked = value;
}

int Screen::id() const
{
    return m_id;
}

void Screen::setId(int id)
{
    m_id = id;
}
